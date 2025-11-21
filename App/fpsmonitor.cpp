#include "fpsmonitor.h"
#include <QDebug>
#include <QFileInfo>
#include <QSettings>
#include <numeric>
#include <QThread>
#include <algorithm>
#include <QColor>
#include "appconstants.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include "RTSSSharedMemory.h"
#include <TlHelp32.h>
#include <psapi.h>
#endif

QString getGradientColor(int index, int total) {
    if (total <= 1) total = 1;
    if (index > total) index = total;

    QColor colorA = QColor::fromHsl(190, 255, 128);
    QColor colorB = QColor::fromHsl(275, 204, 153);

    double ratio = 0.0;
    if (total > 1) {
        ratio = static_cast<double>(index) / static_cast<double>(total - 1);
    }

    int hue = colorA.hue() + (colorB.hue() - colorA.hue()) * ratio;
    int sat = colorA.saturation() + (colorB.saturation() - colorA.saturation()) * ratio;
    int lig = colorA.lightness() + (colorB.lightness() - colorA.lightness()) * ratio;

    QColor finalColor = QColor::fromHsl(hue, sat, lig);

    return finalColor.name().toUpper().remove('#');
}

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
    m_timer->start(500);
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
            QThread::msleep(50);
        } else {
            QThread::msleep(50);
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

            if (exeName.isEmpty() || blacklist.contains(exeName, Qt::CaseInsensitive)) {
                continue;
            }

            uint32_t pid = pAppEntry->dwProcessID;
            currentPids.insert(pid);

            float currentFps = 0.0f;
            if (pAppEntry->dwTime1 > pAppEntry->dwTime0) {
                currentFps = 1000.0f * pAppEntry->dwFrames / (pAppEntry->dwTime1 - pAppEntry->dwTime0);
            }

            if (!m_activeSessions.contains(pid)) {
                if (currentFps >= 1.0f) {
                    QString windowTitle = getWindowTitleByProcessId(pid);
                    GameSessionInfo newSession;
                    newSession.exeName = exeName;
                    newSession.startTime = QDateTime::currentSecsSinceEpoch();
                    m_activeSessions.insert(pid, newSession);
                    emit gameSessionStarted(newSession.exeName, windowTitle, pid);
                } else {
                    currentPids.remove(pid);
                    continue;
                }
            }

            if (m_activeSessions.contains(pid)) {
                m_activeSessions[pid].fpsSamples.append(qRound(currentFps));
                emit activeGameFpsUpdate(pid, qRound(currentFps));

                const QVector<int>& samples = m_activeSessions[pid].fpsSamples;
                double avgFps = 0;
                int minFps = 0;
                int maxFps = 0;

                if (!samples.isEmpty()) {
                    avgFps = std::accumulate(samples.constBegin(), samples.constEnd(), 0.0) / samples.size();
                    minFps = *std::min_element(samples.constBegin(), samples.constEnd());
                    maxFps = *std::max_element(samples.constBegin(), samples.constEnd());
                }

                if (overlayEnabled) {

                    int position = settings.value(AppConfig::SETTING_OVERLAY_POSITION, 0).toInt();
                    bool isBottomPosition = (position == 2 || position == 3);

                    switch (position) {
                    case 0: pAppEntry->dwOSDX = 15; pAppEntry->dwOSDY = 15; break;
                    case 1: pAppEntry->dwOSDX = -15; pAppEntry->dwOSDY = 15; break;
                    case 2: pAppEntry->dwOSDX = 15; pAppEntry->dwOSDY = -15; break;
                    case 3: pAppEntry->dwOSDX = -15; pAppEntry->dwOSDY = -15; break;
                    }

                    const QString cWhite = "<C=FFFFFF>";
                    const QString cLagZeroBlue = "<C=00D1FF>";
                    const QString separator = "";
                    const int labelWidth = 10;
                    const int valueWidth = 5;

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
                    bool showFans = settings.value(AppConfig::SETTING_OVERLAY_SHOW_FANS, true).toBool();
                    bool showMbTemp = settings.value(AppConfig::SETTING_OVERLAY_SHOW_MB_TEMP, true).toBool();
                    bool showStorageTemp = settings.value(AppConfig::SETTING_OVERLAY_SHOW_STORAGE_TEMP, true).toBool();

                    QString valCpuTemp = "--", valCpuUsage = "--", valCpuPower = "--", valCpuClock = "--";
                    QString hardwareCpuName = "PROCESSADOR";
                    if (m_lastHardwareInfo.contains(AppConfig::CPU_KEY)) {
                        const auto& info = m_lastHardwareInfo[AppConfig::CPU_KEY];
                        hardwareCpuName = info.name;
                        if (info.temperature >= 0) valCpuTemp = QString::number(info.temperature, 'f', 0);
                        if (info.usage >= 0) valCpuUsage = QString::number(info.usage, 'f', 0);
                        if (info.power >= 0) valCpuPower = QString::number(info.power, 'f', 0);
                        if (info.clock >= 0) valCpuClock = QString::number(info.clock, 'f', 0);
                    }

                    QString valGpuTemp = "--", valGpuUsage = "--", valGpuPower = "--", valGpuClock = "--";
                    QString hardwareGpuName = "PLACA DE VÍDEO";
                    if (m_lastHardwareInfo.contains(AppConfig::GPU_KEY)) {
                        const auto& info = m_lastHardwareInfo[AppConfig::GPU_KEY];
                        hardwareGpuName = info.name;
                        if (info.temperature >= 0) valGpuTemp = QString::number(info.temperature, 'f', 0);
                        if (info.usage >= 0) valGpuUsage = QString::number(info.usage, 'f', 0);
                        if (info.power >= 0) valGpuPower = QString::number(info.power, 'f', 0);
                        if (info.clock >= 0) valGpuClock = QString::number(info.clock, 'f', 0);
                    }

                    QString valRamUsage = "--";
                    if (m_lastHardwareInfo.contains("RAM_USAGE")) {
                        double usageValue = m_lastHardwareInfo["RAM_USAGE"].usage;
                        if (usageValue >= 0) valRamUsage = QString::number(usageValue, 'f', 0);
                    }

                    QString valMbTemp = "--";
                    if (m_lastHardwareInfo.contains(AppConfig::MB_KEY)) {
                        double tempValue = m_lastHardwareInfo[AppConfig::MB_KEY].temperature;
                        if (tempValue >= 0) valMbTemp = QString::number(tempValue, 'f', 0);
                    }

                    QString valFps = QString::number(qRound(currentFps));
                    QString valAvgFps = QString::number(static_cast<int>(avgFps));
                    QString valMinFps = QString::number(minFps);
                    QString valMaxFps = QString::number(maxFps);

                    auto addMetric = [&](const QString& colorTag, const QString& label, const QString& value, const QString& unit) {
                        return QString("%1%2 %3%4 %5").arg(colorTag, label.leftJustified(labelWidth), cWhite, value.rightJustified(valueWidth), unit.leftJustified(4));
                    };

                    auto addFanMetric = [&](const QString& colorTag, const QString& label, const QString& value) {
                        return QString("%1%2 %3%4 %5").arg(colorTag, label.leftJustified(labelWidth), cWhite, value.rightJustified(valueWidth), QString("RPM").leftJustified(4));
                    };

                    QStringList lines;
                    bool isDetailed = (overlayStyle == 0);

                    if (isDetailed) {
                        lines.append("TITLE_LAGZERO");
                        lines.append(separator);
                    }

                    lines.append("FPS");
                    if (showAvg) lines.append("FPS_AVG");
                    if (showMin) lines.append("FPS_MIN");
                    if (showMax) lines.append("FPS_MAX");

                    bool hasCpuMetrics = showCpuTemp || showCpuUsage || showCpuPower || showCpuClock;
                    if (isDetailed && hasCpuMetrics) {
                        lines.append(separator);
                        lines.append("TITLE_CPU");
                    }
                    if (showCpuTemp) lines.append("CPU_TEMP");
                    if (showCpuUsage) lines.append("CPU_USO");
                    if (showCpuPower) lines.append("CPU_PWR");
                    if (showCpuClock) lines.append("CPU_CLK");

                    bool hasGpuMetrics = showGpuTemp || showGpuUsage || showGpuPower || showGpuClock;
                    if (isDetailed && hasGpuMetrics) {
                        lines.append(separator);
                        lines.append("TITLE_GPU");
                    }
                    if (showGpuTemp) lines.append("GPU_TEMP");
                    if (showGpuUsage) lines.append("GPU_USO");
                    if (showGpuPower) lines.append("GPU_PWR");
                    if (showGpuClock) lines.append("GPU_CLK");

                    bool hasOtherMetrics = showRamUsage || showMbTemp || showStorageTemp;
                    if (isDetailed && hasOtherMetrics) {
                        lines.append(separator);
                        lines.append("TITLE_OUTROS");
                    }
                    if (showRamUsage) lines.append("RAM");
                    if (showMbTemp) lines.append("MB");

                    if (showStorageTemp) {
                        for (auto it = m_lastHardwareInfo.constBegin(); it != m_lastHardwareInfo.constEnd(); ++it) {
                            if (it.key().startsWith(AppConfig::STORAGE_KEY_PREFIX)) {
                                if (it.value().temperature >= 0) {
                                    QString label = it.value().driveType.contains("HD") ? "HD" : "SSD";
                                    lines.append(QString("STORAGE_%1_%2").arg(label).arg(QString::number(it.value().temperature, 'f', 0)));
                                }
                            }
                        }
                    }

                    QStringList fanLines;
                    auto addFanLines = [&](const QMap<QString, double>& fanMap, const QString& prefix) {
                        for(auto it = fanMap.constBegin(); it != fanMap.constEnd(); ++it) {
                            if (it.value() > 0) {
                                QString name = it.key().toUpper();
                                name.remove("CHASSIS ");
                                name.remove("SYSTEM ");
                                name.remove(" FAN");
                                if (name.contains("#")) name = prefix + " " + name.split('#').last();
                                else if (fanMap.size() == 1 && (prefix == "CPU" || prefix == "GPU")) name = prefix;
                                else name = name.left(labelWidth);
                                fanLines.append(QString("FAN_%1_%2").arg(name).arg(QString::number(it.value(), 'f', 0)));
                            }
                        }
                    };

                    if (showFans) {
                        if(m_lastHardwareInfo.contains(AppConfig::CPU_KEY)) addFanLines(m_lastHardwareInfo[AppConfig::CPU_KEY].fans, "CPU");
                        if(m_lastHardwareInfo.contains(AppConfig::GPU_KEY)) addFanLines(m_lastHardwareInfo[AppConfig::GPU_KEY].fans, "GPU");
                        if(m_lastHardwareInfo.contains(AppConfig::MB_KEY)) addFanLines(m_lastHardwareInfo[AppConfig::MB_KEY].fans, "FAN");
                    }

                    if (isDetailed && !fanLines.isEmpty()) lines.append(separator);
                    lines.append(fanLines);

                    QStringList finalLines;
                    int totalLines = lines.count();
                    int lineIndex = 0;

                    for (const QString& line : lines) {
                        QString colorTag = QString("<C=%1>").arg(getGradientColor(lineIndex++, totalLines));

                        if (line == "FPS") finalLines.append(addMetric(colorTag, "FPS", valFps, ""));
                        else if (line == "FPS_AVG") finalLines.append(addMetric(colorTag, "AVG", valAvgFps, ""));
                        else if (line == "FPS_MIN") finalLines.append(addMetric(colorTag, "MIN", valMinFps, ""));
                        else if (line == "FPS_MAX") finalLines.append(addMetric(colorTag, "MAX", valMaxFps, ""));
                        else if (line == "CPU_TEMP") finalLines.append(addMetric(colorTag, isDetailed ? "TEMP" : "CPU T", valCpuTemp, "°C"));
                        else if (line == "CPU_USO") finalLines.append(addMetric(colorTag, isDetailed ? "USO" : "CPU U", valCpuUsage, "%"));
                        else if (line == "CPU_PWR") finalLines.append(addMetric(colorTag, "PWR", valCpuPower, "W"));
                        else if (line == "CPU_CLK") finalLines.append(addMetric(colorTag, "CLK", valCpuClock, "MHz"));
                        else if (line == "GPU_TEMP") finalLines.append(addMetric(colorTag, isDetailed ? "TEMP" : "GPU T", valGpuTemp, "°C"));
                        else if (line == "GPU_USO") finalLines.append(addMetric(colorTag, isDetailed ? "USO" : "GPU U", valGpuUsage, "%"));
                        else if (line == "GPU_PWR") finalLines.append(addMetric(colorTag, "PWR", valGpuPower, "W"));
                        else if (line == "GPU_CLK") finalLines.append(addMetric(colorTag, "CLK", valGpuClock, "MHz"));
                        else if (line == "RAM") finalLines.append(addMetric(colorTag, "RAM", valRamUsage, "MB"));
                        else if (line == "MB") finalLines.append(addMetric(colorTag, "PLACA MAE", valMbTemp, "°C"));
                        else if (line.startsWith("STORAGE_")) {
                            QStringList parts = line.split('_');
                            finalLines.append(addMetric(colorTag, parts[1], parts[2], "°C"));
                        }
                        else if (line.startsWith("FAN_")) {
                            QStringList parts = line.split('_');
                            finalLines.append(addFanMetric(colorTag, parts[1], parts[2]));
                        }
                        else if (line == separator) finalLines.append(separator);
                        else if (line == "TITLE_LAGZERO") finalLines.append(QString("%1LAG ZERO").arg(cLagZeroBlue));
                        else if (line == "TITLE_CPU") finalLines.append(QString("%1%2").arg(colorTag, hardwareCpuName));
                        else if (line == "TITLE_GPU") finalLines.append(QString("%1%2").arg(colorTag, hardwareGpuName));
                        else if (line == "TITLE_OUTROS") finalLines.append(QString("%1OUTROS").arg(colorTag));
                    }

                    if (isBottomPosition) {
                        std::reverse(finalLines.begin(), finalLines.end());
                    }

                    if (!finalLines.isEmpty()) {
                        QString overlayBlock = finalLines.join("\n");
                        QByteArray overlayBytes = overlayBlock.toUtf8();

                        RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY pOSDEntry =
                            (RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY)((LPBYTE)pMem + pMem->dwOSDArrOffset);

                        strcpy_s(pOSDEntry->szOSDOwner, "LAGZERO");
                        strcpy_s(pOSDEntry->szOSD, "");
                        strcpy_s(pOSDEntry->szOSDEx, sizeof(pOSDEntry->szOSDEx), overlayBytes.constData());

                    }
                } else {
                    RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY pOSDEntry =
                        (RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY)((LPBYTE)pMem + pMem->dwOSDArrOffset);
                    if (strcmp(pOSDEntry->szOSDOwner, "LAGZERO") == 0) {
                        strcpy_s(pOSDEntry->szOSDOwner, "");
                        strcpy_s(pOSDEntry->szOSD, "");
                        strcpy_s(pOSDEntry->szOSDEx, "");
                    }
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
