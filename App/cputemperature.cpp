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
    QMap<QString, double> temperatures;
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        QByteArray stdOut = m_process->readAllStandardOutput();
        QString resultStr = QString::fromLatin1(stdOut).trimmed();
        resultStr.replace(',', '.');

        QStringList parts = resultStr.split(';', Qt::SkipEmptyParts);
        for (const QString &part : parts) {
            QStringList pair = part.split(':');
            if (pair.length() == 2) {
                bool ok;
                double temp = pair[1].toDouble(&ok);
                if (ok) {
                    temperatures.insert(pair[0], temp);
                }
            }
        }
    }
    emit temperaturesUpdated(temperatures);
}
