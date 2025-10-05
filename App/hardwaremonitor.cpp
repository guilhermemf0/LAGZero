#include "hardwaremonitor.h"
#include <QDebug>
#include <QCoreApplication>
#include <QFile>
#include <QRegularExpression>

// --- HardwareMonitor (Classe Principal) ---
HardwareMonitor::HardwareMonitor(QObject *parent) : QObject(parent)
{
    HardwareWorker *worker = new HardwareWorker();
    worker->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(&workerThread, &QThread::started, worker, &HardwareWorker::process);
    connect(worker, &HardwareWorker::hardwareUpdated, this, &HardwareMonitor::hardwareUpdated);
    connect(worker, &HardwareWorker::helperMissing, this, &HardwareMonitor::helperMissing);
    workerThread.start();
}

HardwareMonitor::~HardwareMonitor() {
    if(workerThread.isRunning()){
        workerThread.quit();
        workerThread.wait();
    }
}


// --- HardwareWorker (Lógica em Thread Separada) ---
HardwareWorker::HardwareWorker() : m_process(nullptr), m_timer(nullptr) {}

HardwareWorker::~HardwareWorker()
{
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(1000);
    }
}

void HardwareWorker::process() {
    QString programPath = QCoreApplication::applicationDirPath() + "/TempReader.exe";
    if (!QFile::exists(programPath)) {
        emit helperMissing();
        return;
    }

    m_process = new QProcess(this);
    connect(m_process, &QProcess::finished, this, &HardwareWorker::onProcessFinished);
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &HardwareWorker::readHardwareData);
    m_timer->start(2000);
    readHardwareData();
}
void HardwareWorker::readHardwareData() {
    if (m_process && m_process->state() == QProcess::NotRunning) {
        QString appDir = QCoreApplication::applicationDirPath();
        QString program = appDir + "/TempReader.exe";
        m_process->setWorkingDirectory(appDir);
        m_process->start(program, QStringList());
    }
}
void HardwareWorker::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    QMap<QString, HardwareInfo> deviceInfos;
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        QByteArray stdOut = m_process->readAllStandardOutput();
        QString resultStr = QString::fromLatin1(stdOut).trimmed();
        resultStr.replace(',', '.');

        QStringList parts = resultStr.split(';', Qt::SkipEmptyParts);
        for (const QString &part : parts) {
            QStringList pair = part.split(':');
            if (pair.length() < 4) continue;

            QString fullKey = pair[0];
            QString hardwareName = pair[1];
            QString sensorName = pair[2];
            double value = pair[3].toDouble();

            HardwareInfo info;
            info.name = hardwareName;

            if (fullKey.startsWith("CPU_TEMPERATURE")) {
                if (sensorName.contains("Package") || sensorName.contains("Tctl/Tdie)") || sensorName == "No Sensor") {
                    info.temperature = value;
                    deviceInfos.insert("CPU", info);
                }
                else if (sensorName.contains("Core #")) {
                    QString coreId = QString(sensorName).remove(QRegularExpression("[^0-9]"));
                    info.temperature = value;
                    deviceInfos.insert("CPU_CORE_" + coreId, info);
                }
            } else if (fullKey.startsWith("CPU_LOAD")) {
                if (sensorName.contains("CPU Total")) {
                    info.usage = value;
                    deviceInfos.insert("CPU_USAGE", info);
                }
            } else if (fullKey.startsWith("GPU_TEMPERATURE")) {
                if (sensorName.contains("Hotspot") || sensorName.contains("Core") || sensorName == "No Sensor") {
                    if (!deviceInfos.contains("GPU")) {
                        info.temperature = value;
                        deviceInfos.insert("GPU", info);
                    }
                }
            } else if (fullKey.startsWith("GPU_LOAD")) {
                if (sensorName.contains("GPU Core") || sensorName.contains("D3D 3D")){
                    info.usage = value;
                    deviceInfos.insert("GPU_USAGE", info);
                }
            } else if (fullKey.startsWith("RAM_DATA")) {
                if (sensorName == "Memory Used") { // Alterado de .contains() para == para ser exato
                    info.usage = value;
                    deviceInfos.insert("RAM_USAGE", info);
                }
            } else if (fullKey.startsWith("MB_TEMPERATURE")) {
                if (!deviceInfos.contains("MOTHERBOARD")) {
                    info.temperature = value;
                    deviceInfos.insert("MOTHERBOARD", info);
                }
            } else if (fullKey.startsWith("STORAGE") && fullKey.contains("TEMPERATURE")) {
                info.temperature = value;
                if (fullKey.contains("SSD")) info.driveType = "SSD";
                else if (fullKey.contains("HD")) info.driveType = "HD";
                deviceInfos.insert(fullKey, info);
            }
        }
    }
    emit hardwareUpdated(deviceInfos);
}
