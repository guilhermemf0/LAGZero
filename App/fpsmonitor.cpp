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
        const QList<QString> blacklist = {"App.exe", "SensorReader.exe", "devenv.exe", "msedgewebview2.exe", "TabTip.exe"};
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
                QString alignTag = "";
                switch (position) {
                case 0: pAppEntry->dwOSDX = 15; pAppEntry->dwOSDY = 15; break;
                case 1: pAppEntry->dwOSDX = -15; pAppEntry->dwOSDY = 15; alignTag = "<R>"; break;
                case 2: pAppEntry->dwOSDX = 15; pAppEntry->dwOSDY = -15; break;
                case 3: pAppEntry->dwOSDX = -15; pAppEntry->dwOSDY = -15; alignTag = "<R>"; break;
                }

                int overlayStyle = settings.value(AppConfig::SETTING_OVERLAY_STYLE, 0).toInt();

                bool showAvg = settings.value(AppConfig::SETTING_OVERLAY_SHOW_AVG_FPS, true).toBool();
                bool showMin = settings.value(AppConfig::SETTING_OVERLAY_SHOW_MIN_FPS, true).toBool();
                bool showMax = settings.value(AppConfig::SETTING_OVERLAY_SHOW_MAX_FPS, true).toBool();
                bool showCpuUsage = settings.value(AppConfig::SETTING_OVERLAY_SHOW_CPU_USAGE, true).toBool();
                bool showCpuTemp = settings.value(AppConfig::SETTING_OVERLAY_SHOW_CPU_TEMP, true).toBool();
                bool showCpuPower = settings.value(AppConfig::SETTING_OVERLAY_SHOW_CPU_POWER, true).toBool();
                bool showCpuClock = settings.value(AppConfig::SETTING_OVERLAY_SHOW_CPU_CLOCK, true).toBool();
                bool showGpuUsage = settings.value(AppConfig::SETTING_OVERLAY_SHOW_GPU_USAGE, true).toBool();
                bool showGpuTemp = settings.value(AppConfig::SETTING_OVERLAY_SHOW_GPU_TEMP, true).toBool();
                bool showGpuPower = settings.value(AppConfig::SETTING_OVERLAY_SHOW_GPU_POWER, true).toBool();
                bool showGpuClock = settings.value(AppConfig::SETTING_OVERLAY_SHOW_GPU_CLOCK, true).toBool();
                bool showRamUsage = settings.value(AppConfig::SETTING_OVERLAY_SHOW_RAM_USAGE, true).toBool();
                bool showCores = settings.value(AppConfig::SETTING_OVERLAY_SHOW_CPU_CORES, false).toBool();
                bool showMb = settings.value(AppConfig::SETTING_OVERLAY_SHOW_MB_TEMP, false).toBool();
                bool showStorage = settings.value(AppConfig::SETTING_OVERLAY_SHOW_STORAGE_TEMP, false).toBool();
                bool showFans = settings.value(AppConfig::SETTING_OVERLAY_SHOW_FANS, true).toBool();

                QString cpuUsage = "--";
                QString cpuTemp = "--";
                QString cpuPower = "--";
                QString cpuClock = "--";
                QString cpuName = "CPU";
                if (m_lastHardwareInfo.contains(AppConfig::CPU_KEY)) {
                    const auto& info = m_lastHardwareInfo[AppConfig::CPU_KEY];
                    cpuName = info.name.split(' ').first();
                    if (info.usage >= 0) cpuUsage = QString::number(info.usage, 'f', 0);
                    if (info.temperature >= 0) cpuTemp = QString::number(info.temperature, 'f', 0);
                    if (info.power >= 0) cpuPower = QString::number(info.power, 'f', 0);
                    if (info.clock >= 0) cpuClock = QString::number(info.clock, 'f', 0);
                }

                QString gpuUsage = "--";
                QString gpuTemp = "--";
                QString gpuPower = "--";
                QString gpuClock = "--";
                QString gpuName = "GPU";
                if (m_lastHardwareInfo.contains(AppConfig::GPU_KEY)) {
                    const auto& info = m_lastHardwareInfo[AppConfig::GPU_KEY];
                    gpuName = info.name.split(' ').last();
                    if (info.usage >= 0) gpuUsage = QString::number(info.usage, 'f', 0);
                    if (info.temperature >= 0) gpuTemp = QString::number(info.temperature, 'f', 0);
                    if (info.power >= 0) gpuPower = QString::number(info.power, 'f', 0);
                    if (info.clock >= 0) gpuClock = QString::number(info.clock, 'f', 0);
                }

                QString ramUsage = "--";
                if (m_lastHardwareInfo.contains("RAM_USAGE")) {
                    double usageValue = m_lastHardwareInfo["RAM_USAGE"].usage;
                    if (usageValue >= 0) ramUsage = QString::number(usageValue / 1024.0, 'f', 1); // Em GB
                }

                QStringList overlayLines;
                const QString cWhite = "<C=FFFFFF>";
                const QString cGray = "<C=94A3B8>";
                const QString cBlue = "<C=00D1FF>";
                const QString cCpu = "<C=FF00FF>";
                const QString cGpu = "<C=00FF00>";
                const QString cRam = "<C=FFFF00>";

                if (overlayStyle == 0) {
                    overlayLines.append(alignTag + cBlue + "LAG ZERO");

                    QStringList fpsStats;
                    if (showAvg) fpsStats.append(QString("%1AVG %2%3").arg(cGray, cWhite, QString::number(static_cast<int>(avgFps)).rightJustified(3)));
                    if (showMin) fpsStats.append(QString("%1MIN %2%3").arg(cGray, cWhite, QString::number(minFps).rightJustified(3)));
                    if (showMax) fpsStats.append(QString("%1MAX %2%3").arg(cGray, cWhite, QString::number(maxFps).rightJustified(3)));
                    overlayLines.append(alignTag + QString("%1FPS %2%3 %4")
                                                       .arg(cGray)
                                                       .arg(cWhite)
                                                       .arg(QString::number(qRound(currentFps)).rightJustified(3))
                                                       .arg(fpsStats.join(" ")));

                    overlayLines.append(alignTag + cGray + "--------------------");

                    if (showCpuUsage || showCpuTemp || showCpuPower || showCpuClock) {
                        overlayLines.append(alignTag + QString("%1%2").arg(cCpu, cpuName));
                        QString line1, line2;
                        if (showCpuUsage) line1.append(QString("%1USO %2%3%   ").arg(cGray, cWhite, cpuUsage.rightJustified(3)));
                        if (showCpuPower) line1.append(QString("%1PWR %2%3W").arg(cGray, cWhite, cpuPower.rightJustified(3)));
                        if (showCpuTemp) line2.append(QString("%1TEMP %2%3C   ").arg(cGray, cWhite, cpuTemp.rightJustified(3)));
                        if (showCpuClock) line2.append(QString("%1CLK %2%3MHz").arg(cGray, cWhite, cpuClock.rightJustified(4)));
                        if (!line1.isEmpty()) overlayLines.append(alignTag + line1);
                        if (!line2.isEmpty()) overlayLines.append(alignTag + line2);
                    }
                    if (showGpuUsage || showGpuTemp || showGpuPower || showGpuClock) {
                        overlayLines.append(alignTag + QString("%1%2").arg(cGpu, gpuName));
                        QString line1, line2;
                        if (showGpuUsage) line1.append(QString("%1USO %2%3%   ").arg(cGray, cWhite, gpuUsage.rightJustified(3)));
                        if (showGpuPower) line1.append(QString("%1PWR %2%3W").arg(cGray, cWhite, gpuPower.rightJustified(3)));
                        if (showGpuTemp) line2.append(QString("%1TEMP %2%3C   ").arg(cGray, cWhite, gpuTemp.rightJustified(3)));
                        if (showGpuClock) line2.append(QString("%1CLK %2%3MHz").arg(cGray, cWhite, gpuClock.rightJustified(4)));
                        if (!line1.isEmpty()) overlayLines.append(alignTag + line1);
                        if (!line2.isEmpty()) overlayLines.append(alignTag + line2);
                    }
                    if (showRamUsage) {
                        overlayLines.append(alignTag + QString("%1RAM %2%3 GB").arg(cRam, cWhite).arg(ramUsage.rightJustified(4)));
                    }

                    if (showCores && m_lastHardwareInfo.contains(AppConfig::CPU_KEY)) {
                        const auto& coreMap = m_lastHardwareInfo[AppConfig::CPU_KEY].coreTemps;
                        QStringList coreTemps;
                        for(auto it = coreMap.constBegin(); it != coreMap.constEnd(); ++it) {
                            QString temp = (it.value() >= 0) ? QString::number(it.value(), 'f', 0) : "--";
                            QString coreName = it.key().split(' ').last().remove('#');
                            coreTemps.append(QString("%1C%2 %3%4C").arg(cCpu, coreName, cWhite).arg(temp.rightJustified(3)));
                        }
                        if (!coreTemps.isEmpty()) {
                            overlayLines.append(alignTag + cGray + "--------------------");
                            for(int j = 0; j < coreTemps.size(); j += 2) {
                                QString line = coreTemps[j].leftJustified(10);
                                if (j + 1 < coreTemps.size()) {
                                    line.append(QString(" %1| %2").arg(cGray).arg(coreTemps[j+1]));
                                }
                                overlayLines.append(alignTag + line);
                            }
                        }
                    }

                    QStringList tempsLines;
                    if (showMb && m_lastHardwareInfo.contains(AppConfig::MB_KEY)) {
                        double tempValue = m_lastHardwareInfo[AppConfig::MB_KEY].temperature;
                        QString temp = (tempValue >= 0) ? QString::number(tempValue, 'f', 0) : "--";
                        tempsLines.append(QString("%1Placa Mae %2%3 C").arg(cGray, cWhite).arg(temp.rightJustified(3)));
                    }
                    if (showStorage) {
                        for(auto it = m_lastHardwareInfo.constBegin(); it != m_lastHardwareInfo.constEnd(); ++it) {
                            if (it.key().startsWith(AppConfig::STORAGE_KEY_PREFIX)) {
                                double tempValue = it.value().temperature;
                                QString temp = (tempValue >= 0) ? QString::number(tempValue, 'f', 0) : "--";
                                QString label = it.value().driveType.contains("HD") ? "HD" : "SSD";
                                tempsLines.append(QString("%1%2 %3%4 C").arg(cGray, label.leftJustified(9)).arg(cWhite).arg(temp.rightJustified(3)));
                            }
                        }
                    }
                    if (!tempsLines.isEmpty()) {
                        overlayLines.append(alignTag + cGray + "--------------------");
                        overlayLines.append(tempsLines);
                    }

                    if (showFans) {
                        QStringList fansLines;
                        auto addFanLines = [&](const QMap<QString, double>& fanMap, const QString& sourcePrefix) {
                            for(auto it = fanMap.constBegin(); it != fanMap.constEnd(); ++it) {
                                if (it.value() > 0) {
                                    QString name = sourcePrefix + it.key();
                                    if (name.length() > 10) name = name.left(10);
                                    QString speed = QString::number(it.value(), 'f', 0);
                                    fansLines.append(QString("%1%2 %3%4 RPM").arg(cGray, name.leftJustified(10)).arg(cWhite).arg(speed.rightJustified(4)));
                                }
                            }
                        };
                        if (m_lastHardwareInfo.contains(AppConfig::CPU_KEY)) addFanLines(m_lastHardwareInfo[AppConfig::CPU_KEY].fans, "CPU ");
                        if (m_lastHardwareInfo.contains(AppConfig::GPU_KEY)) addFanLines(m_lastHardwareInfo[AppConfig::GPU_KEY].fans, "GPU ");
                        if (m_lastHardwareInfo.contains(AppConfig::MB_KEY)) addFanLines(m_lastHardwareInfo[AppConfig::MB_KEY].fans, "");

                        if (!fansLines.isEmpty()) {
                            overlayLines.append(alignTag + cGray + "--------------------");
                            overlayLines.append(fansLines);
                        }
                    }
                }
                else if (overlayStyle == 1)
                {
                    QStringList fpsLine;
                    fpsLine.append(QString("%1FPS: %2%3").arg(cGray, cWhite, QString::number(qRound(currentFps))));
                    if (showAvg) fpsLine.append(QString("%1AVG %2%3").arg(cGray, cWhite, QString::number(static_cast<int>(avgFps))));
                    if (showMin) fpsLine.append(QString("%1MIN %2%3").arg(cGray, cWhite, QString::number(minFps)));
                    if (showMax) fpsLine.append(QString("%1MAX %2%3").arg(cGray, cWhite, QString::number(maxFps)));
                    overlayLines.append(alignTag + fpsLine.join(QString(" %1| ").arg(cGray)));

                    QStringList cpuParts;
                    if (showCpuUsage) cpuParts.append(QString("%1%2%").arg(cWhite, cpuUsage));
                    if (showCpuTemp) cpuParts.append(QString("%1%2C").arg(cWhite, cpuTemp));
                    if (showCpuPower) cpuParts.append(QString("%1%2W").arg(cWhite, cpuPower));
                    if (!cpuParts.isEmpty()) overlayLines.append(alignTag + QString("%1CPU: ").arg(cCpu) + cpuParts.join(QString(" %1| ").arg(cGray)));

                    QStringList gpuParts;
                    if (showGpuUsage) gpuParts.append(QString("%1%2%").arg(cWhite, gpuUsage));
                    if (showGpuTemp) gpuParts.append(QString("%1%2C").arg(cWhite, gpuTemp));
                    if (showGpuPower) gpuParts.append(QString("%1%2W").arg(cWhite, gpuPower));
                    if (!gpuParts.isEmpty()) overlayLines.append(alignTag + QString("%1GPU: ").arg(cGpu) + gpuParts.join(QString(" %1| ").arg(cGray)));

                    if (showRamUsage) {
                        overlayLines.append(alignTag + QString("%1RAM: %2%3 GB").arg(cRam, cWhite, ramUsage));
                    }

                    if (showFans) {
                        QStringList fanParts;
                        auto addFanParts = [&](const QMap<QString, double>& fanMap, const QString& sourcePrefix) {
                            for(auto it = fanMap.constBegin(); it != fanMap.constEnd(); ++it) {
                                if (it.value() > 0) {
                                    QString speed = QString::number(it.value(), 'f', 0);
                                    fanParts.append(QString("%1%2 %3RPM").arg(cGray, sourcePrefix).arg(cWhite, speed));
                                }
                            }
                        };

                        QMap<QString, double> cpuFans = m_lastHardwareInfo.value(AppConfig::CPU_KEY).fans;
                        QMap<QString, double> gpuFans = m_lastHardwareInfo.value(AppConfig::GPU_KEY).fans;
                        QMap<QString, double> mbFans = m_lastHardwareInfo.value(AppConfig::MB_KEY).fans;

                        addFanParts(cpuFans, "CPU:");
                        addFanParts(gpuFans, "GPU:");

                        for(auto it = mbFans.constBegin(); it != mbFans.constEnd(); ++it) {
                            if (it.value() > 0 && !cpuFans.contains(it.key()) && !gpuFans.contains(it.key())) {
                                QString speed = QString::number(it.value(), 'f', 0);
                                fanParts.append(QString("%1SYS: %2%3 RPM").arg(cGray).arg(cWhite, speed));
                            }
                        }

                        if (!fanParts.isEmpty()) {
                            overlayLines.append(alignTag + fanParts.join(QString(" %1| ").arg(cGray)));
                        }
                    }
                }

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
