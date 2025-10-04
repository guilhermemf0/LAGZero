#include "fpsmonitor.h"
#include <QDebug>
#include <QFileInfo>
#include <QSettings>
#include <numeric>
#include <QThread>
#include "appconstants.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include "RTSSSharedMemory.h"
#include <TlHelp32.h>
#include <psapi.h>
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

    connect(this, &FpsMonitor::hardwareDataUpdated, worker, &FpsWorker::onHardwareUpdated);

    workerThread.start();
}

FpsMonitor::~FpsMonitor()
{
    if(workerThread.isRunning()) {
        workerThread.quit();
        workerThread.wait();
    }
}

void FpsMonitor::onHardwareUpdated(const QMap<QString, HardwareInfo> &deviceInfos)
{
    emit hardwareDataUpdated(deviceInfos);
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

void FpsWorker::onHardwareUpdated(const QMap<QString, HardwareInfo> &deviceInfos)
{
    m_lastHardwareInfo = deviceInfos;
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

struct EnumData {
    DWORD processId;
    HWND bestHnd;
    int bestTitleLength;
};

BOOL CALLBACK EnumWindowsCallback(HWND hwnd, LPARAM lParam)
{
    EnumData* data = (EnumData*)lParam;
    DWORD processId = 0;
    GetWindowThreadProcessId(hwnd, &processId);

    if (data->processId == processId && IsWindowVisible(hwnd)) {
        int length = GetWindowTextLength(hwnd);
        if (length > data->bestTitleLength) {
            wchar_t title[256];
            GetWindowTextW(hwnd, title, 256);
            QString qTitle = QString::fromWCharArray(title);
            if (qTitle != "D3DProxyWindow" && !qTitle.contains("NVIDIA") && !qTitle.contains("AMD") && qTitle.length() > 3) {
                data->bestHnd = hwnd;
                data->bestTitleLength = length;
            }
        }
    }
    return TRUE;
}

QString FpsWorker::getWindowTitleByProcessId(DWORD processId)
{
    EnumData data = { processId, nullptr, 0 };
    for (int i = 0; i < 10; ++i) {
        EnumWindows(EnumWindowsCallback, (LPARAM)&data);
        if (data.bestHnd) {
            QThread::msleep(500);
        } else {
            QThread::msleep(500);
        }
    }

    if (data.bestHnd) {
        wchar_t title[256];
        GetWindowTextW(data.bestHnd, title, 256);
        return QString::fromWCharArray(title);
    }
    return QString();
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

    HANDLE hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, "RTSSSharedMemoryV2");
    if (!hMapFile) return;

    LPVOID pMapAddr = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (!pMapAddr) {
        CloseHandle(hMapFile);
        return;
    }

    LPRTSS_SHARED_MEMORY pMem = (LPRTSS_SHARED_MEMORY)pMapAddr;

    if ((pMem->dwSignature == 0x52545353) && (pMem->dwVersion >= 0x00020000))
    {
        QSettings settings("LAGZero", "MonitorApp");
        bool overlayEnabled = settings.value(AppConfig::SETTING_OVERLAY_ENABLED, true).toBool();

        QSet<uint32_t> currentPids;
        const QList<QString> blacklist = {"App.exe", "TempReader.exe", "devenv.exe", "msedgewebview2.exe", "TabTip.exe"};

        DWORD currentTime = GetTickCount();

        for (DWORD i = 0; i < pMem->dwAppArrSize; ++i)
        {
            LPBYTE basePtr = (LPBYTE)pMem + pMem->dwAppArrOffset;
            RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_APP_ENTRY pAppEntry =
                (RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_APP_ENTRY)(basePtr + i * pMem->dwAppEntrySize);

            QString exeName = QString::fromLocal8Bit(pAppEntry->szName);

            if (exeName.isEmpty() || blacklist.contains(exeName, Qt::CaseInsensitive) || pAppEntry->dwTime1 <= pAppEntry->dwTime0)
            {
                continue;
            }

            float currentFps = 1000.0f * pAppEntry->dwFrames / (pAppEntry->dwTime1 - pAppEntry->dwTime0);
            if (currentFps < 1.0f) continue;

            uint32_t pid = pAppEntry->dwProcessID;
            currentPids.insert(pid);

            if (!m_activeSessions.contains(pid)) {
                if (currentTime > pAppEntry->dwTime1 && (currentTime - pAppEntry->dwTime1) > 2000) {
                    continue;
                }

                QString windowTitle = getWindowTitleByProcessId(pid);
                qDebug() << "Novo jogo detectado:" << exeName << "PID:" << pid << "Título:" << windowTitle;

                GameSessionInfo newSession;
                newSession.exeName = exeName;
                newSession.startTime = QDateTime::currentSecsSinceEpoch();
                m_activeSessions.insert(pid, newSession);
                emit gameSessionStarted(newSession.exeName, windowTitle, pid);
            }

            m_activeSessions[pid].fpsSamples.append(qRound(currentFps));
            emit activeGameFpsUpdate(pid, qRound(currentFps));

            if (overlayEnabled) {
                int position = settings.value(AppConfig::SETTING_OVERLAY_POSITION, 0).toInt();
                switch (position) {
                case 0: pAppEntry->dwOSDX = 5; pAppEntry->dwOSDY = 5; break; // TL
                case 1: pAppEntry->dwOSDX = -5; pAppEntry->dwOSDY = 5; break; // TR
                case 2: pAppEntry->dwOSDX = 5; pAppEntry->dwOSDY = -5; break; // BL
                case 3: pAppEntry->dwOSDX = -5; pAppEntry->dwOSDY = -5; break; // BR
                }

                QStringList overlayLines;
                overlayLines.append(QString("<C=00FFFF>FPS<C=FFFFFF>: %1").arg(qRound(currentFps)));

                bool usageShown = false;
                if (settings.value(AppConfig::SETTING_OVERLAY_SHOW_CPU_USAGE, true).toBool() && m_lastHardwareInfo.contains("CPU_USAGE")) {
                    overlayLines.append(QString("<C=FF00FF>CPU Usage<C=FFFFFF>: %1%").arg(m_lastHardwareInfo["CPU_USAGE"].usage, 0, 'f', 1));
                    usageShown = true;
                }
                if (settings.value(AppConfig::SETTING_OVERLAY_SHOW_GPU_USAGE, true).toBool() && m_lastHardwareInfo.contains("GPU_USAGE")) {
                    overlayLines.append(QString("<C=00FF00>GPU Usage<C=FFFFFF>: %1%").arg(m_lastHardwareInfo["GPU_USAGE"].usage, 0, 'f', 1));
                    usageShown = true;
                }
                if (settings.value(AppConfig::SETTING_OVERLAY_SHOW_RAM_USAGE, true).toBool() && m_lastHardwareInfo.contains("RAM_USAGE")) {
                    overlayLines.append(QString("<C=FFFF00>RAM Usage<C=FFFFFF>: %1 MB").arg(m_lastHardwareInfo["RAM_USAGE"].usage, 0, 'f', 0));
                    usageShown = true;
                }

                if (usageShown) {
                    overlayLines.append(QString("<C=808080>--------------------"));
                }

                if (settings.value(AppConfig::SETTING_OVERLAY_SHOW_CPU_TEMP, true).toBool() && m_lastHardwareInfo.contains(AppConfig::CPU_KEY)) {
                    overlayLines.append(QString::fromWCharArray(L"<C=FF00FF>CPU Temp<C=FFFFFF>: %1°C").arg(m_lastHardwareInfo[AppConfig::CPU_KEY].temperature, 0, 'f', 0));
                }

                if (settings.value(AppConfig::SETTING_OVERLAY_SHOW_CPU_CORES, true).toBool()) {
                    QStringList coreTemps;
                    for (auto it = m_lastHardwareInfo.constBegin(); it != m_lastHardwareInfo.constEnd(); ++it) {
                        if (it.key().startsWith("CPU_CORE_")) {
                            coreTemps.append(QString::fromWCharArray(L"<C=FF00FF>%1<C=FFFFFF>: %2°C").arg(it.value().name).arg(it.value().temperature, 0, 'f', 0));
                        }
                    }
                    if (!coreTemps.isEmpty()) {
                        for(int i = 0; i < coreTemps.size(); i += 2) {
                            QString line = coreTemps[i];
                            if (i + 1 < coreTemps.size()) {
                                line += "\t| " + coreTemps[i+1];
                            }
                            overlayLines.append(line);
                        }
                    }
                }

                if (settings.value(AppConfig::SETTING_OVERLAY_SHOW_GPU_TEMP, true).toBool() && m_lastHardwareInfo.contains(AppConfig::GPU_KEY)) {
                    overlayLines.append(QString::fromWCharArray(L"<C=00FF00>GPU Temp<C=FFFFFF>: %1°C").arg(m_lastHardwareInfo[AppConfig::GPU_KEY].temperature, 0, 'f', 0));
                }
                if (settings.value(AppConfig::SETTING_OVERLAY_SHOW_MB_TEMP, false).toBool() && m_lastHardwareInfo.contains(AppConfig::MB_KEY)) {
                    overlayLines.append(QString::fromWCharArray(L"<C=FFFF00>MB Temp<C=FFFFFF>: %1°C").arg(m_lastHardwareInfo[AppConfig::MB_KEY].temperature, 0, 'f', 0));
                }
                if (settings.value(AppConfig::SETTING_OVERLAY_SHOW_STORAGE_TEMP, true).toBool()) { // Também ajustei o padrão para 'true'
                    for(auto it = m_lastHardwareInfo.constBegin(); it != m_lastHardwareInfo.constEnd(); ++it) {
                        if (it.key().startsWith(AppConfig::STORAGE_KEY_PREFIX) && it.value().temperature >= 0) {
                            overlayLines.append(QString("<C=FFA500>%1 Temp<C=FFFFFF>: %2°C").arg(it.value().driveType).arg(it.value().temperature, 0, 'f', 0));
                        }
                    }
                }

                RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY pOSDEntry =
                    (RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY)((LPBYTE)pMem + pMem->dwOSDArrOffset);

                QByteArray overlayBytes = overlayLines.join("\n").toUtf8();
                strcpy_s(pOSDEntry->szOSDOwner, "LAGZERO");

                strcpy_s(pOSDEntry->szOSDEx, "");
                strcpy_s(pOSDEntry->szOSDEx, sizeof(pOSDEntry->szOSDEx), overlayBytes.constData());

            } else { // Se o overlay estiver desabilitado, limpa
                RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY pOSDEntry =
                    (RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY)((LPBYTE)pMem + pMem->dwOSDArrOffset);
                if (strcmp(pOSDEntry->szOSDOwner, "LAGZERO") == 0) {
                    strcpy_s(pOSDEntry->szOSDOwner, "");
                    strcpy_s(pOSDEntry->szOSD, "");
                    strcpy_s(pOSDEntry->szOSDEx, "");
                }
            }
            pMem->dwOSDFrame++;
        }

        QList<uint32_t> closedPids;
        for (auto it = m_activeSessions.begin(); it != m_activeSessions.end(); ++it) {
            if (!currentPids.contains(it.key())) {
                closedPids.append(it.key());
            }
        }

        for (uint32_t pid : closedPids) {
            GameSessionInfo session = m_activeSessions.take(pid);

            RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY pOSDEntry =
                (RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY)((LPBYTE)pMem + pMem->dwOSDArrOffset);

            if (strcmp(pOSDEntry->szOSDOwner, "LAGZERO") == 0) {
                strcpy_s(pOSDEntry->szOSDOwner, "");
                strcpy_s(pOSDEntry->szOSD, "");
                strcpy_s(pOSDEntry->szOSDEx, "");
                pMem->dwOSDFrame++;
            }

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

