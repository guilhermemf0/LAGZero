#include "fpsmonitor.h"
#include <QTimer>
// #include <QDebug> // Removido, pois não é mais necessário
#include <QRegularExpression>
#include <QFileInfo>
#include <QSettings> // Adicionado para ler o registro do Windows

#ifdef Q_OS_WIN
#include <windows.h>
#include "RTSSSharedMemory.h"
#endif

// Construtor da classe FpsMonitor, inicia o worker em uma nova thread.
FpsMonitor::FpsMonitor(QObject *parent) : QObject(parent)
{
    FpsWorker *worker = new FpsWorker();
    worker->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(&workerThread, &QThread::started, worker, &FpsWorker::process);
    connect(worker, &FpsWorker::fpsUpdated, this, &FpsMonitor::fpsUpdated);
    // NOVO: Conecta o sinal rtssStatusUpdated do worker para a classe principal
    connect(worker, &FpsWorker::rtssStatusUpdated, this, &FpsMonitor::rtssStatusUpdated);
    workerThread.start();
}

// Destrutor da classe FpsMonitor, garante que a thread seja finalizada.
FpsMonitor::~FpsMonitor()
{
    if(workerThread.isRunning()) {
        workerThread.quit();
        workerThread.wait();
    }
}

// Construtor do worker.
FpsWorker::FpsWorker() {}

// Destrutor do worker, libera os recursos de memória mapeada.
FpsWorker::~FpsWorker()
{
    if (m_pMapFile) UnmapViewOfFile(m_pMapFile);
    if (m_hMapFile) CloseHandle(m_hMapFile);
}

void FpsWorker::process()
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &FpsWorker::readFps);
    m_timer->start(100);
    readFps();
}

// NOVO: Função para verificar se o RTSS está instalado e rodando
bool FpsWorker::isRtssRunning()
{
    // Tenta abrir o mapeamento de arquivo do RTSS
    HANDLE hMapFile = OpenFileMappingA(FILE_MAP_READ, FALSE, "RTSSSharedMemoryV2");
    if (hMapFile) {
        CloseHandle(hMapFile); // Fecha o handle imediatamente
        return true;
    }
    return false;
}

// NOVO: Função para obter o caminho de instalação do RTSS do registro
QString FpsWorker::getRtssInstallPath()
{
    QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Unwinder\\RTSS", QSettings::NativeFormat);
    return settings.value("InstallPath").toString();
}

// Função para ler o FPS diretamente das entradas de aplicativo do RTSS.
void FpsWorker::readFps()
{
#ifdef Q_OS_WIN
    QString appName = "";
    int fpsValue = 0;
    bool rtssFound = false;

    // NOVO: Verifica se o RTSS está rodando
    if (!isRtssRunning()) {
        emit rtssStatusUpdated(false, getRtssInstallPath());
        emit fpsUpdated(0, "");
        return;
    } else {
        rtssFound = true;
        emit rtssStatusUpdated(true, getRtssInstallPath());
    }

    HANDLE hMapFile = OpenFileMappingA(FILE_MAP_READ, FALSE, "RTSSSharedMemoryV2");
    if (!hMapFile) {
        emit fpsUpdated(0, "");
        return;
    }

    LPVOID pMapAddr = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
    if (!pMapAddr) {
        CloseHandle(hMapFile);
        emit fpsUpdated(0, "");
        return;
    }

    LPRTSS_SHARED_MEMORY pMem = (LPRTSS_SHARED_MEMORY)pMapAddr;

    if ((pMem->dwSignature == 0x52545353) && (pMem->dwVersion >= 0x00020000))
    {
        // Itera sobre as entradas de aplicativo (arrApp) para encontrar o FPS
        for (DWORD i = 0; i < pMem->dwAppArrSize; ++i)
        {
            // Calcula o ponteiro para a entrada do aplicativo
            LPBYTE basePtr = (LPBYTE)pMem + pMem->dwAppArrOffset;
            if ((i * pMem->dwAppEntrySize) >= (pMem->dwAppArrSize * pMem->dwAppEntrySize) && pMem->dwAppArrSize > 0) {
                break;
            }
            RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_APP_ENTRY pAppEntry =
                (RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_APP_ENTRY)(basePtr + i * pMem->dwAppEntrySize);

            // Verifica se é um aplicativo válido (com nome e tempos de frame)
            if (strlen(pAppEntry->szName) > 0 && pAppEntry->dwTime1 > pAppEntry->dwTime0)
            {
                float currentFps = 1000.0f * pAppEntry->dwFrames / (pAppEntry->dwTime1 - pAppEntry->dwTime0);
                QString fullPathAppName = QString::fromLocal8Bit(pAppEntry->szName);

                // Extrai apenas o nome do arquivo do caminho completo
                QFileInfo fileInfo(fullPathAppName);
                QString currentAppName = fileInfo.baseName(); // Pega o nome do arquivo sem a extensão

                // Formata o nome do aplicativo (substitui '_' por espaço e capitaliza palavras)
                currentAppName.replace('_', ' '); // Substitui underscores por espaços
                bool capitalizeNext = true;
                for (int j = 0; j < currentAppName.length(); ++j) {
                    if (capitalizeNext && currentAppName[j].isLetter()) {
                        currentAppName[j] = currentAppName[j].toUpper();
                        capitalizeNext = false;
                    } else if (currentAppName[j].isSpace()) {
                        capitalizeNext = true;
                    }
                }

                // Filtra para garantir que o FPS é plausível e o nome não é de um processo de sistema
                if (currentFps > 1.0f &&
                    !currentAppName.contains("explorer", Qt::CaseInsensitive) &&
                    !currentAppName.contains("dwm", Qt::CaseInsensitive) &&
                    !currentAppName.contains("csrss", Qt::CaseInsensitive))
                {
                    // Se o RTSS reporta o último aplicativo em primeiro plano, podemos usar isso.
                    // Caso contrário, pegamos o primeiro aplicativo válido com FPS.
                    if (pMem->dwVersion >= 0x00020016 && pMem->dwLastForegroundApp == i) {
                        appName = currentAppName;
                        fpsValue = qRound(currentFps);
                        break; // Encontramos o FPS do aplicativo em primeiro plano
                    } else if (appName.isEmpty()) { // Se ainda não encontramos um, pegamos o primeiro válido
                        appName = currentAppName;
                        fpsValue = qRound(currentFps);
                    }
                }
            }
        }

        if (fpsValue > 0 && !appName.isEmpty()) {
            emit fpsUpdated(fpsValue, appName);
        } else {
            emit fpsUpdated(0, "");
        }

    } else {
        emit fpsUpdated(0, "");
    }

    UnmapViewOfFile(pMapAddr);
    CloseHandle(hMapFile);

#else
    emit fpsUpdated(0, "N/A (Windows only)");
#endif
}
