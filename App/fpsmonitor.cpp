#include "fpsmonitor.h"
#include <QDebug>
#include <QFileInfo>
#include <QSettings>
#include <numeric>

#ifdef Q_OS_WIN
#include <windows.h>
#include "RTSSSharedMemory.h"
#endif

// --- FpsMonitor (Classe Principal) ---
FpsMonitor::FpsMonitor(QObject *parent) : QObject(parent)
{
    FpsWorker *worker = new FpsWorker();
    worker->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(&workerThread, &QThread::started, worker, &FpsWorker::process);

    connect(worker, &FpsWorker::rtssStatusUpdated, this, &FpsMonitor::rtssStatusUpdated);
    connect(worker, &FpsWorker::gameSessionStarted, this, &FpsMonitor::gameSessionStarted);
    connect(worker, &FpsWorker::gameSessionEnded, this, &FpsMonitor::gameSessionEnded);
    connect(worker, &FpsWorker::activeGameFpsUpdate, this, &FpsMonitor::activeGameFpsUpdate);

    workerThread.start();
}

FpsMonitor::~FpsMonitor()
{
    if(workerThread.isRunning()) {
        workerThread.quit();
        workerThread.wait();
    }
}


// --- FpsWorker (Lógica em Thread Separada) ---
FpsWorker::FpsWorker() {}

FpsWorker::~FpsWorker() {}

void FpsWorker::process()
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &FpsWorker::readFps);
    m_timer->start(1000);
    readFps();
}

bool FpsWorker::isRtssRunning()
{
    HANDLE hMapFile = OpenFileMappingA(FILE_MAP_READ, FALSE, "RTSSSharedMemoryV2");
    if (hMapFile) {
        CloseHandle(hMapFile);
        return true;
    }
    return false;
}

QString FpsWorker::getRtssInstallPath()
{
    QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Unwinder\\RTSS", QSettings::NativeFormat);
    return settings.value("InstallPath").toString();
}

void FpsWorker::readFps()
{
#ifdef Q_OS_WIN
    if (!isRtssRunning()) {
        if (!m_activeSessions.isEmpty()) {
            for (auto it = m_activeSessions.begin(); it != m_activeSessions.end(); ++it) {
                double avg = 0;
                if (!it.value().fpsSamples.isEmpty()) {
                    avg = std::accumulate(it.value().fpsSamples.constBegin(), it.value().fpsSamples.constEnd(), 0.0) / it.value().fpsSamples.size();
                }
                emit gameSessionEnded(it.key(), it.value().exeName, avg);
            }
            m_activeSessions.clear();
        }
        emit rtssStatusUpdated(false, getRtssInstallPath());
        return;
    }

    emit rtssStatusUpdated(true, getRtssInstallPath());

    HANDLE hMapFile = OpenFileMappingA(FILE_MAP_READ, FALSE, "RTSSSharedMemoryV2");
    if (!hMapFile) return;

    LPVOID pMapAddr = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
    if (!pMapAddr) {
        CloseHandle(hMapFile);
        return;
    }

    LPRTSS_SHARED_MEMORY pMem = (LPRTSS_SHARED_MEMORY)pMapAddr;

    if ((pMem->dwSignature == 0x52545353) && (pMem->dwVersion >= 0x00020000))
    {
        QSet<uint32_t> currentPids;

        // CORREÇÃO: Lista de processos a serem ignorados para evitar falsos positivos
        const QList<QString> blacklist = {"App.exe", "TempReader.exe", "devenv.exe", "msedgewebview2.exe"};

        for (DWORD i = 0; i < pMem->dwAppArrSize; ++i)
        {
            LPBYTE basePtr = (LPBYTE)pMem + pMem->dwAppArrOffset;
            RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_APP_ENTRY pAppEntry =
                (RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_APP_ENTRY)(basePtr + i * pMem->dwAppEntrySize);

            QString exeName = QString::fromLocal8Bit(pAppEntry->szName);

            // Verifica se o processo está na blacklist ou se não tem dados válidos
            if (exeName.isEmpty() || blacklist.contains(exeName, Qt::CaseInsensitive) || pAppEntry->dwTime1 <= pAppEntry->dwTime0)
            {
                continue;
            }

            float currentFps = 1000.0f * pAppEntry->dwFrames / (pAppEntry->dwTime1 - pAppEntry->dwTime0);
            if (currentFps < 1.0f) continue;

            uint32_t pid = pAppEntry->dwProcessID;
            currentPids.insert(pid);

            if (!m_activeSessions.contains(pid)) {
                GameSessionInfo newSession;
                newSession.exeName = exeName;
                newSession.startTime = QDateTime::currentSecsSinceEpoch();
                m_activeSessions.insert(pid, newSession);
                emit gameSessionStarted(newSession.exeName, pid);
            }

            m_activeSessions[pid].fpsSamples.append(qRound(currentFps));
            emit activeGameFpsUpdate(pid, qRound(currentFps));
        }

        QList<uint32_t> closedPids;
        for (auto it = m_activeSessions.begin(); it != m_activeSessions.end(); ++it) {
            if (!currentPids.contains(it.key())) {
                closedPids.append(it.key());
            }
        }

        for (uint32_t pid : closedPids) {
            GameSessionInfo session = m_activeSessions.take(pid);
            double avg = 0;
            if (!session.fpsSamples.isEmpty()) {
                avg = std::accumulate(session.fpsSamples.constBegin(), session.fpsSamples.constEnd(), 0.0) / session.fpsSamples.size();
            }
            emit gameSessionEnded(pid, session.exeName, avg);
        }
    }

    UnmapViewOfFile(pMapAddr);
    CloseHandle(hMapFile);

#else
    // Lógica para outros sistemas operacionais (não implementado)
#endif
}
