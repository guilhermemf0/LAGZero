#include "hardwaremonitor.h"
#include <QDebug>
#include <QCoreApplication>
#include <QFile>

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

// CORREÇÃO FINAL: Garante que o processo seja encerrado de forma limpa
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
    m_timer->start(2000); // Aumentado para 2 segundos para dar tempo de resposta
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
    emit hardwareUpdated(deviceInfos);
}
