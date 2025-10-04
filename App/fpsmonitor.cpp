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

FpsWorker::FpsWorker() {}

FpsWorker::~FpsWorker()
{
    HANDLE hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, "RTSSSharedMemoryV2");
    if (!hMapFile) return;

    LPVOID pMapAddr = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (pMapAddr) {
        LPRTSS_SHARED_MEMORY pMem = (LPRTSS_SHARED_MEMORY)pMapAddr;
        if ((pMem->dwSignature == 0x52545353) && (pMem->dwVersion >= 0x00020000))
        {
            RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY pOSDEntry =
                (RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY)((LPBYTE)pMem + pMem->dwOSDArrOffset);
            if (strcmp(pOSDEntry->szOSDOwner, "LAGZERO") == 0) {
                strcpy_s(pOSDEntry->szOSDOwner, "");
                strcpy_s(pOSDEntry->szOSD, "");
                strcpy_s(pOSDEntry->szOSDEx, "");
                pMem->dwOSDFrame++;
            }
        }
        UnmapViewOfFile(pMapAddr);
    }
    CloseHandle(hMapFile);
}

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
            const QVector<int>& samples = m_activeSessions[pid].fpsSamples;
            double avgFps = std::accumulate(samples.constBegin(), samples.constEnd(), 0.0) / samples.size();
            int minFps = *std::min_element(samples.constBegin(), samples.constEnd());
            int maxFps = *std::max_element(samples.constBegin(), samples.constEnd());

            if (overlayEnabled) {
                int position = settings.value(AppConfig::SETTING_OVERLAY_POSITION, 0).toInt();
                switch (position) {
                case 0: pAppEntry->dwOSDX = 15; pAppEntry->dwOSDY = 15; break;
                case 1: pAppEntry->dwOSDX = -15; pAppEntry->dwOSDY = 15; break;
                case 2: pAppEntry->dwOSDX = 15; pAppEntry->dwOSDY = -15; break;
                case 3: pAppEntry->dwOSDX = -15; pAppEntry->dwOSDY = -15; break;
                }

                bool showAvg = settings.value(AppConfig::SETTING_OVERLAY_SHOW_AVG_FPS, true).toBool();
                bool showMin = settings.value(AppConfig::SETTING_OVERLAY_SHOW_MIN_FPS, true).toBool();
                bool showMax = settings.value(AppConfig::SETTING_OVERLAY_SHOW_MAX_FPS, true).toBool();
                bool showCpuUsage = settings.value(AppConfig::SETTING_OVERLAY_SHOW_CPU_USAGE, true).toBool();
                bool showCpuTemp = settings.value(AppConfig::SETTING_OVERLAY_SHOW_CPU_TEMP, true).toBool();
                bool showGpuUsage = settings.value(AppConfig::SETTING_OVERLAY_SHOW_GPU_USAGE, true).toBool();
                bool showGpuTemp = settings.value(AppConfig::SETTING_OVERLAY_SHOW_GPU_TEMP, true).toBool();
                bool showRamUsage = settings.value(AppConfig::SETTING_OVERLAY_SHOW_RAM_USAGE, true).toBool();
                bool showCores = settings.value(AppConfig::SETTING_OVERLAY_SHOW_CPU_CORES, false).toBool();
                bool showMb = settings.value(AppConfig::SETTING_OVERLAY_SHOW_MB_TEMP, false).toBool();
                bool showStorage = settings.value(AppConfig::SETTING_OVERLAY_SHOW_STORAGE_TEMP, false).toBool();

                QStringList overlayLines;
                QStringList mainHardwareLines;
                QStringList temperaturesLines;

                overlayLines.append("<C=00D1FF>+- LAG ZERO ---------------+");
                overlayLines.append(QString("<C=00D1FF>|<C=FFFFFF> FPS %1").arg(QString::number(qRound(currentFps)).rightJustified(4)));
                QString statsLine = "<C=00D1FF>| ";
                if (showAvg) statsLine.append(QString("<C=94A3B8>AVG<C=FFFFFF>%1 ").arg(QString::number(static_cast<int>(avgFps)).rightJustified(4)));
                if (showMin) statsLine.append(QString("<C=94A3B8>MIN<C=FFFFFF>%1 ").arg(QString::number(minFps).rightJustified(4)));
                if (showMax) statsLine.append(QString("<C=94A3B8>MAX<C=FFFFFF>%1").arg(QString::number(maxFps).rightJustified(4)));
                if (showAvg || showMin || showMax) {
                    overlayLines.append(statsLine);
                }

                if (showCpuUsage) {
                    QString usage = m_lastHardwareInfo.contains("CPU_USAGE") ? QString::number(m_lastHardwareInfo["CPU_USAGE"].usage, 'f', 0) : "--";
                    QString line = QString("<C=00D1FF>|<C=FF00FF> CPU   <C=FFFFFF>%1%").arg(usage.rightJustified(3));
                    if (showCpuTemp) {
                        QString temp = m_lastHardwareInfo.contains(AppConfig::CPU_KEY) ? QString::number(m_lastHardwareInfo[AppConfig::CPU_KEY].temperature, 'f', 0) : "--";
                        line.append(QString("<C=808080> |<C=FFFFFF> %1<C=808080> C").arg(temp.rightJustified(3)));
                    }
                    mainHardwareLines.append(line);
                } else if (showCpuTemp) {
                    QString temp = m_lastHardwareInfo.contains(AppConfig::CPU_KEY) ? QString::number(m_lastHardwareInfo[AppConfig::CPU_KEY].temperature, 'f', 0) : "--";
                    temperaturesLines.append(QString("<C=00D1FF>|<C=FF00FF> CPU       <C=FFFFFF>%1<C=808080> C").arg(temp.rightJustified(3)));
                }

                if (showGpuUsage) {
                    QString usage = m_lastHardwareInfo.contains("GPU_USAGE") ? QString::number(m_lastHardwareInfo["GPU_USAGE"].usage, 'f', 0) : "--";
                    QString line = QString("<C=00D1FF>|<C=00FF00> GPU   <C=FFFFFF>%1%").arg(usage.rightJustified(3));
                    if (showGpuTemp) {
                        QString temp = m_lastHardwareInfo.contains(AppConfig::GPU_KEY) ? QString::number(m_lastHardwareInfo[AppConfig::GPU_KEY].temperature, 'f', 0) : "--";
                        line.append(QString("<C=808080> |<C=FFFFFF> %1<C=808080> C").arg(temp.rightJustified(3)));
                    }
                    mainHardwareLines.append(line);
                } else if (showGpuTemp) {
                    QString temp = m_lastHardwareInfo.contains(AppConfig::GPU_KEY) ? QString::number(m_lastHardwareInfo[AppConfig::GPU_KEY].temperature, 'f', 0) : "--";
                    temperaturesLines.append(QString("<C=00D1FF>|<C=00FF00> GPU       <C=FFFFFF>%1<C=808080> C").arg(temp.rightJustified(3)));
                }

                if (showRamUsage) {
                    QString usage = m_lastHardwareInfo.contains("RAM_USAGE") ? QString::number(m_lastHardwareInfo["RAM_USAGE"].usage, 'f', 0) : "--";
                    mainHardwareLines.append(QString("<C=00D1FF>|<C=FFFF00> RAM   <C=FFFFFF>%1 MB").arg(usage.rightJustified(5)));
                }

                if (!mainHardwareLines.isEmpty()) {
                    overlayLines.append("<C=00D1FF>+- HARDWARE ---------------+");
                    overlayLines.append(mainHardwareLines);
                }

                if (showCores) {
                    QStringList coreTemps;
                    for (auto it = m_lastHardwareInfo.constBegin(); it != m_lastHardwareInfo.constEnd(); ++it) {
                        if (it.key().startsWith("CPU_CORE_")) {
                            QString temp = it.value().temperature >= 0 ? QString::number(it.value().temperature, 'f', 0) : "--";
                            coreTemps.append(QString("<C=FF00FF>%1<C=FFFFFF> %2<C=808080> C").arg(it.value().name).arg(temp));
                        }
                    }
                    if (!coreTemps.isEmpty()) {
                        overlayLines.append("<C=00D1FF>|..........................|");
                        for(int i = 0; i < coreTemps.size(); i += 2) {
                            QString line = QString("<C=00D1FF>|<C=FFFFFF> %1").arg(coreTemps[i].leftJustified(11));
                            if (i + 1 < coreTemps.size()) {
                                line.append(QString(" <C=808080>|<C=FFFFFF> %1").arg(coreTemps[i+1]));
                            }
                            overlayLines.append(line);
                        }
                    }
                }

                if (showMb) {
                    QString temp = m_lastHardwareInfo.contains(AppConfig::MB_KEY) ? QString::number(m_lastHardwareInfo[AppConfig::MB_KEY].temperature, 'f', 0) : "--";
                    temperaturesLines.append(QString("<C=00D1FF>|<C=94A3B8> Placa Mae   <C=FFFFFF>%1<C=808080> C").arg(temp.rightJustified(3)));
                }

                if (showStorage) {
                    QStringList ssdTemps, hddTemps;
                    int ssdCount = 1, hddCount = 1;
                    for(auto it = m_lastHardwareInfo.constBegin(); it != m_lastHardwareInfo.constEnd(); ++it) {
                        if (it.key().startsWith(AppConfig::STORAGE_KEY_PREFIX)) {
                            QString temp = it.value().temperature >= 0 ? QString::number(it.value().temperature, 'f', 0) : "--";
                            if (it.value().driveType == "SSD") {
                                QString line = QString("<C=00D1FF>|<C=94A3B8> SSD %1      <C=FFFFFF>%2<C=808080> C").arg(ssdCount++).arg(temp.rightJustified(3));
                                ssdTemps.append(line);
                            } else {
                                QString line = QString("<C=00D1FF>|<C=94A3B8> HDD %1      <C=FFFFFF>%2<C=808080> C").arg(hddCount++).arg(temp.rightJustified(3));
                                hddTemps.append(line);
                            }
                        }
                    }
                    if (!ssdTemps.isEmpty()) { temperaturesLines.append(ssdTemps); }
                    if (!hddTemps.isEmpty()) { temperaturesLines.append(hddTemps); }
                }

                if (!temperaturesLines.isEmpty()) {
                    overlayLines.append("<C=00D1FF>+- TEMPERATURAS -----------+");
                    overlayLines.append(temperaturesLines);
                }

                overlayLines.append("<C=00D1FF>+--------------------------+");

                RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY pOSDEntry =
                    (RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY)((LPBYTE)pMem + pMem->dwOSDArrOffset);
                QByteArray overlayBytes = overlayLines.join("\n").toUtf8();
                strcpy_s(pOSDEntry->szOSDOwner, "LAGZERO");
                strcpy_s(pOSDEntry->szOSD, "");
                strcpy_s(pOSDEntry->szOSDEx, sizeof(pOSDEntry->szOSDEx), overlayBytes.constData());

            } else {
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
#endif
}
