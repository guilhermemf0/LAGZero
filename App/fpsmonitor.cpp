#include "fpsmonitor.h"
#include <QTimer>
#include <QDebug>
#include <QRegularExpression> // Incluído para uso no .cpp

#ifdef Q_OS_WIN
#include <windows.h> // Necessário para APIs de Windows
#include "RTSSSharedMemory.h" // AGORA INCLUÍMOS O ARQUIVO OFICIAL DO RTSS
#endif

// Construtor da classe FpsMonitor, inicia o worker em uma nova thread.
FpsMonitor::FpsMonitor(QObject *parent) : QObject(parent)
{
    FpsWorker *worker = new FpsWorker(); // Cria o worker
    worker->moveToThread(&workerThread); // Move o worker para a thread
    // Conecta o sinal finished da thread para deletar o worker quando a thread terminar
    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    // MUDADO: Conecta o sinal started da thread para chamar a função process() diretamente
    // Não é mais um slot, então usamos um cast para chamar a função membro.
    connect(&workerThread, &QThread::started, worker, &FpsWorker::process);
    // Conecta o sinal fpsUpdated do worker para re-emitir na classe principal
    connect(worker, &FpsWorker::fpsUpdated, this, &FpsMonitor::fpsUpdated);
    workerThread.start(); // Inicia a thread
}

// Destrutor da classe FpsMonitor, garante que a thread seja finalizada.
FpsMonitor::~FpsMonitor()
{
    if(workerThread.isRunning()) {
        workerThread.quit(); // Pede para a thread terminar
        workerThread.wait(); // Espera a thread realmente parar
    }
}

// Construtor do worker.
FpsWorker::FpsWorker() {}

// Destrutor do worker, libera os recursos de memória mapeada.
FpsWorker::~FpsWorker()
{
    // Apenas desmapeia e fecha o handle, não precisamos mais liberar um slot OSD customizado
    if (m_pMapFile) UnmapViewOfFile(m_pMapFile);
    if (m_hMapFile) CloseHandle(m_hMapFile);
}

// MUDADO: process() não é mais um slot, é uma função membro pública comum.
void FpsWorker::process()
{
    m_timer = new QTimer(this); // Cria um QTimer
    connect(m_timer, &QTimer::timeout, this, &FpsWorker::readFps); // Conecta o timeout para chamar readFps
    m_timer->start(100); // MUDADO: Roda a cada 100 milissegundos para atualização mais rápida
    readFps(); // Chama imediatamente na primeira vez
}


// Função para ler o FPS diretamente das entradas de aplicativo do RTSS.
void FpsWorker::readFps()
{
    qDebug() << "[FPS Worker] readFps() chamado."; // DEBUG
#ifdef Q_OS_WIN
    QString appName = "";
    int fpsValue = 0;

    HANDLE hMapFile = OpenFileMappingA(FILE_MAP_READ, FALSE, "RTSSSharedMemoryV2");
    if (!hMapFile) {
        qDebug() << "[FPS Worker] Falha ao abrir mapeamento de arquivo RTSSSharedMemoryV2.";
        emit fpsUpdated(0, "");
        return;
    }

    LPVOID pMapAddr = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
    if (!pMapAddr) {
        qDebug() << "[FPS Worker] Falha ao mapear a visão do arquivo RTSSSharedMemoryV2.";
        CloseHandle(hMapFile);
        emit fpsUpdated(0, "");
        return;
    }

    LPRTSS_SHARED_MEMORY pMem = (LPRTSS_SHARED_MEMORY)pMapAddr;

    if ((pMem->dwSignature == 0x52545353) && (pMem->dwVersion >= 0x00020000))
    {
        qDebug() << "[FPS Worker] Assinatura RTSS e versão V" << HIWORD(pMem->dwVersion) << "." << LOWORD(pMem->dwVersion) << " válidas.";

        // Itera sobre as entradas de aplicativo (arrApp) para encontrar o FPS
        for (DWORD i = 0; i < pMem->dwAppArrSize; ++i)
        {
            // Calcula o ponteiro para a entrada do aplicativo
            LPBYTE basePtr = (LPBYTE)pMem + pMem->dwAppArrOffset;
            if ((i * pMem->dwAppEntrySize) >= (pMem->dwAppArrSize * pMem->dwAppEntrySize) && pMem->dwAppArrSize > 0) {
                qDebug() << "[FPS Worker] Índice de aplicativo fora dos limites. i:" << i;
                break;
            }
            RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_APP_ENTRY pAppEntry =
                (RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_APP_ENTRY)(basePtr + i * pMem->dwAppEntrySize);

            // Verifica se é um aplicativo válido (com nome e tempos de frame)
            if (strlen(pAppEntry->szName) > 0 && pAppEntry->dwTime1 > pAppEntry->dwTime0)
            {
                float currentFps = 1000.0f * pAppEntry->dwFrames / (pAppEntry->dwTime1 - pAppEntry->dwTime0);
                QString currentAppName = QString::fromLocal8Bit(pAppEntry->szName);

                // Filtra para garantir que o FPS é plausível e o nome não é de um processo de sistema
                // MUDADO: Removido o limite superior de FPS
                if (currentFps > 1.0f &&
                    !currentAppName.contains("explorer.exe", Qt::CaseInsensitive) &&
                    !currentAppName.contains("dwm.exe", Qt::CaseInsensitive) &&
                    !currentAppName.contains("csrss.exe", Qt::CaseInsensitive))
                {
                    // Se o RTSS reporta o último aplicativo em primeiro plano, podemos usar isso.
                    // Caso contrário, pegamos o primeiro aplicativo válido com FPS.
                    if (pMem->dwVersion >= 0x00020016 && pMem->dwLastForegroundApp == i) {
                        appName = currentAppName;
                        fpsValue = qRound(currentFps);
                        qDebug() << "[FPS Worker] FPS encontrado para aplicativo em primeiro plano. App:" << appName << ", FPS:" << fpsValue;
                        break; // Encontramos o FPS do aplicativo em primeiro plano
                    } else if (appName.isEmpty()) { // Se ainda não encontramos um, pegamos o primeiro válido
                        appName = currentAppName;
                        fpsValue = qRound(currentFps);
                        qDebug() << "[FPS Worker] FPS encontrado para primeiro aplicativo válido. App:" << appName << ", FPS:" << fpsValue;
                    }
                }
            }
        }

        if (fpsValue > 0 && !appName.isEmpty()) {
            emit fpsUpdated(fpsValue, appName);
        } else {
            qDebug() << "[FPS Worker] Nenhum FPS válido encontrado em arrApp. Emitindo N/A.";
            emit fpsUpdated(0, "");
        }

    } else {
        qDebug() << "[FPS Worker] Assinatura RTSS (" << QString::number(pMem->dwSignature, 16) << ") ou versão (" << QString::number(pMem->dwVersion, 16) << ") INVÁLIDA.";
        emit fpsUpdated(0, "");
    }

    UnmapViewOfFile(pMapAddr);
    CloseHandle(hMapFile);

#else
    qDebug() << "[FPS Worker] Sistema operacional não-Windows. Emitindo N/A.";
    emit fpsUpdated(0, "N/A (Windows only)");
#endif
}
