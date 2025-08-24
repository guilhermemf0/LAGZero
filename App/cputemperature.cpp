#include "cputemperature.h"
#include <QDebug>
#include <QCoreApplication>

CpuTemperature::CpuTemperature(QObject *parent) : QObject(parent)
{
    TemperatureWorker *worker = new TemperatureWorker();
    worker->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(&workerThread, &QThread::started, worker, &TemperatureWorker::process);
    connect(worker, &TemperatureWorker::temperaturesUpdated, this, &CpuTemperature::temperaturesUpdated);
    workerThread.start();
}
CpuTemperature::~CpuTemperature() { if(workerThread.isRunning()){ workerThread.quit(); workerThread.wait(); } }

TemperatureWorker::TemperatureWorker() {}
void TemperatureWorker::process() {
    m_process = new QProcess(this);
    connect(m_process, &QProcess::finished, this, &TemperatureWorker::onProcessFinished);
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &TemperatureWorker::readTemperature);
    m_timer->start(1000);
    readTemperature();
}
void TemperatureWorker::readTemperature() {
    if (m_process->state() == QProcess::NotRunning) {
        QString appDir = QCoreApplication::applicationDirPath();
        QString program = appDir + "/TempReader.exe";
        m_process->setWorkingDirectory(appDir);
        m_process->start(program, QStringList());
    }
}
void TemperatureWorker::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    QMap<QString, HardwareInfo> deviceInfos;
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        QByteArray stdOut = m_process->readAllStandardOutput();
        QString resultStr = QString::fromLatin1(stdOut).trimmed();
        resultStr.replace(',', '.');

        QStringList parts = resultStr.split(';', Qt::SkipEmptyParts);
        for (const QString &part : parts) {
            // --- CORREÇÃO: Interpreta os novos formatos ---
            QStringList pair = part.split(':');
            if (pair.length() < 3) continue;

            QString key = pair[0];
            HardwareInfo info;
            bool ok = false;

            if (key.startsWith("STORAGE") && pair.length() == 4) {
                // Formato Storage: STORAGE_KEY:NAME:TYPE:TEMP
                info.name = pair[1];
                info.driveType = pair[2];
                info.temperature = pair[3].toDouble(&ok);
            } else if (pair.length() == 3) {
                // Formato Padrão: KEY:NAME:TEMP
                info.name = pair[1];
                info.temperature = pair[2].toDouble(&ok);
            }

            if (ok) {
                deviceInfos.insert(key, info);
            }
        }
    }
    emit temperaturesUpdated(deviceInfos);
}
