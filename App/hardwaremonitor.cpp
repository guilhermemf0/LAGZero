#include "hardwaremonitor.h"
#include <QDebug>
#include <QCoreApplication>
#include <QFile>
#include <QRegularExpression>

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

HardwareWorker::HardwareWorker() : m_process(nullptr), m_timer(nullptr) {}

HardwareWorker::~HardwareWorker()
{
    if (m_timer) {
        m_timer->stop();
    }

    if (m_process && m_process->state() != QProcess::NotRunning) {
        disconnect(m_process, &QProcess::finished, this, &HardwareWorker::onProcessFinished);
        m_process->kill();
        m_process->waitForFinished(1000);
    }
}

void HardwareWorker::process() {
    QString programPath = QCoreApplication::applicationDirPath() + "/SensorReader.exe";
    if (!QFile::exists(programPath)) {
        qDebug() << "ERRO: SensorReader.exe não encontrado em" << programPath;
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
        QString program = appDir + "/SensorReader.exe";
        m_process->setWorkingDirectory(appDir);

        QStringList args;
        args << "--once" << "--format" << "Json";

        m_process->start(program, args);
    }
}

void HardwareWorker::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    QMap<QString, HardwareInfo> deviceInfos;

    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        QByteArray stdOut = m_process->readAllStandardOutput();
        QJsonDocument doc = QJsonDocument::fromJson(stdOut);

        if (!doc.isObject()) {
            qWarning() << "Falha ao analisar o JSON do SensorReader. Saída:" << QString::fromLatin1(stdOut);
            emit hardwareUpdated(deviceInfos);
            return;
        }

        QJsonObject report = doc.object();

        QJsonArray cpus = report["Cpus"].toArray();
        if (!cpus.isEmpty()) {
            QJsonObject cpu = cpus.first().toObject();
            HardwareInfo cpuInfo;
            cpuInfo.name = cpu["Name"].toString();

            QJsonArray sensors = cpu["Sensors"].toArray();
            for (const QJsonValue &sensorVal : sensors) {
                QJsonObject sensor = sensorVal.toObject();
                QString type = sensor["Type"].toString();
                QString name = sensor["Name"].toString();
                QString nameLower = name.toLower();
                double value = sensor["Value"].toDouble(-1.0);
                if (value < 0) continue;

                if (type == "Temperature" && (nameLower.contains("package") || nameLower.contains("tctl/tdie"))) {
                    cpuInfo.temperature = value;
                } else if (type == "Temperature" && nameLower.contains("core #")) {
                    cpuInfo.coreTemps.insert(name, value);
                } else if (type == "Load" && nameLower.contains("total")) {
                    cpuInfo.usage = value;
                } else if (type == "Power" && nameLower.contains("package")) {
                    cpuInfo.power = value;
                } else if (type == "Clock" && nameLower.contains("core #")) {
                    cpuInfo.coreClocks.insert(name, value);
                    if (cpuInfo.clock < 0) cpuInfo.clock = value;
                } else if (type == "Fan") {
                    cpuInfo.fans.insert(name, value);
                }
            }
            deviceInfos.insert("CPU", cpuInfo);
            deviceInfos.insert("CPU_USAGE", cpuInfo);
        }

        QJsonArray gpus = report["Gpus"].toArray();
        if (!gpus.isEmpty()) {
            QJsonObject gpu = gpus.first().toObject();
            HardwareInfo gpuInfo;
            gpuInfo.name = gpu["Name"].toString();

            QJsonArray sensors = gpu["Sensors"].toArray();
            for (const QJsonValue &sensorVal : sensors) {
                QJsonObject sensor = sensorVal.toObject();
                QString type = sensor["Type"].toString();
                QString name = sensor["Name"].toString();
                QString nameLower = name.toLower();
                double value = sensor["Value"].toDouble(-1.0);
                if (value < 0) continue;

                if (type == "Temperature" && (nameLower.contains("core") || nameLower.contains("hotspot"))) {
                    if (gpuInfo.temperature < 0 || nameLower.contains("core")) {
                        gpuInfo.temperature = value;
                    }
                } else if (type == "Load" && (nameLower.contains("gpu core") || nameLower.contains("d3d 3d"))) {
                    gpuInfo.usage = value;
                } else if (type == "Power" && nameLower.contains("package")) {
                    gpuInfo.power = value;
                } else if (type == "Clock" && nameLower.contains("core")) {
                    gpuInfo.clock = value;
                } else if (type == "Fan") {
                    gpuInfo.fans.insert(name, value);
                }
            }
            deviceInfos.insert("GPU", gpuInfo);
            deviceInfos.insert("GPU_USAGE", gpuInfo);
        }

        QJsonObject memory = report["Memory"].toObject();
        QJsonArray ramSensors = memory["GlobalSensors"].toArray();
        HardwareInfo ramInfo;
        ramInfo.name = "Memory";
        for (const QJsonValue &sensorVal : ramSensors) {
            QJsonObject sensor = sensorVal.toObject();
            if (sensor["Type"].toString() == "Data" && sensor["Name"].toString() == "Memory Used") {
                ramInfo.usage = sensor["Value"].toDouble(-1.0) * 1024.0;
                break;
            }
        }
        deviceInfos.insert("RAM_USAGE", ramInfo);

        QJsonObject mb = report["Motherboard"].toObject();
        HardwareInfo mbInfo;
        mbInfo.name = mb["Product"].toString();
        QJsonArray mbSensors = mb["Sensors"].toArray();
        double bestTemp = -1.0;
        int bestPriority = 0;

        for (const QJsonValue &sensorVal : mbSensors) {
            QJsonObject sensor = sensorVal.toObject();
            QString type = sensor["Type"].toString();
            QString name = sensor["Name"].toString();
            QString nameLower = name.toLower();
            double value = sensor["Value"].toDouble(-1.0);
            if (value < 0) continue;

            if (type == "Temperature") {
                if (nameLower == "system" && bestPriority < 3) {
                    bestTemp = value; bestPriority = 3;
                } else if (nameLower == "pch" && bestPriority < 2) {
                    bestTemp = value; bestPriority = 2;
                } else if (bestPriority < 1 && !nameLower.contains("cpu")) {
                    bestTemp = value; bestPriority = 1;
                }
            } else if (type == "Fan") {
                mbInfo.fans.insert(name, value);
            }
        }

        if (bestTemp < 0) {
            for (const QJsonValue &sensorVal : mbSensors) {
                QJsonObject sensor = sensorVal.toObject();
                if (sensor["Type"].toString() == "Temperature") {
                    bestTemp = sensor["Value"].toDouble(-1.0);
                    if (bestTemp >= 0) break;
                }
            }
        }
        mbInfo.temperature = bestTemp;
        deviceInfos.insert("MOTHERBOARD", mbInfo);

        QJsonArray storageDevices = report["StorageDevices"].toArray();
        int storageIndex = 0;
        for (const QJsonValue &driveVal : storageDevices) {
            QJsonObject drive = driveVal.toObject();
            HardwareInfo driveInfo;
            driveInfo.name = drive["Model"].toString();
            driveInfo.driveType = drive["MediaType"].toString();
            QString key = "STORAGE_" + QString::number(storageIndex++);

            QJsonArray driveSensors = drive["Sensors"].toArray();
            for (const QJsonValue &sensorVal : driveSensors) {
                QJsonObject sensor = sensorVal.toObject();
                if (sensor["Type"].toString() == "Temperature") {
                    double value = sensor["Value"].toDouble(-1.0);
                    if (value >= 0) {
                        driveInfo.temperature = value;
                        break;
                    }
                }
            }
            deviceInfos.insert(key, driveInfo);
        }
    } else {
        qWarning() << "Processo SensorReader.exe falhou. Código:" << exitCode << "Status:" << exitStatus;
        qWarning() << "Erro:" << m_process->readAllStandardError();
    }

    emit hardwareUpdated(deviceInfos);
}
