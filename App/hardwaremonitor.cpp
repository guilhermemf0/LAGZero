#include "hardwaremonitor.h" // Atualizado
#include <QDebug>
#include <QCoreApplication>

// --- RENOMEADO ---
HardwareMonitor::HardwareMonitor(QObject *parent) : QObject(parent)
{
    HardwareWorker *worker = new HardwareWorker(); // Atualizado
    worker->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(&workerThread, &QThread::started, worker, &HardwareWorker::process); // Atualizado
    connect(worker, &HardwareWorker::hardwareUpdated, this, &HardwareMonitor::hardwareUpdated); // Atualizado
    workerThread.start();
}
HardwareMonitor::~HardwareMonitor() { if(workerThread.isRunning()){ workerThread.quit(); workerThread.wait(); } }

// --- RENOMEADO ---
HardwareWorker::HardwareWorker() {}
void HardwareWorker::process() {
    m_process = new QProcess(this);
    connect(m_process, &QProcess::finished, this, &HardwareWorker::onProcessFinished); // Atualizado
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &HardwareWorker::readHardwareData); // Atualizado
    m_timer->start(1000); // Roda a cada segundo
    readHardwareData(); // Roda imediatamente na primeira vez
}
void HardwareWorker::readHardwareData() { // RENOMEADO
    if (m_process->state() == QProcess::NotRunning) {
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
            if (pair.length() < 3) continue;

            QString key = pair[0];
            HardwareInfo info;
            bool ok = false;

            if (key.startsWith("STORAGE") && pair.length() == 4) {
                info.name = pair[1];
                info.driveType = pair[2];
                info.temperature = pair[3].toDouble(&ok);
            } else if (pair.length() == 3) {
                info.name = pair[1];
                info.temperature = pair[2].toDouble(&ok);
            }

            if (ok) {
                deviceInfos.insert(key, info);
            }
        }
    }
    emit hardwareUpdated(deviceInfos); // RENOMEADO
}
