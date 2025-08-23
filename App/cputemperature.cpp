#include "cputemperature.h"
#include <QDebug>
#include <QCoreApplication>

// ===================================================================
// === Implementação da Classe Principal (Controladora)
// ===================================================================

CpuTemperature::CpuTemperature(QObject *parent) : QObject(parent)
{
    TemperatureWorker *worker = new TemperatureWorker();
    worker->moveToThread(&workerThread);

    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(&workerThread, &QThread::started, worker, &TemperatureWorker::process);
    connect(worker, &TemperatureWorker::temperatureUpdated, this, &CpuTemperature::temperatureUpdated);

    workerThread.start();
}

CpuTemperature::~CpuTemperature()
{
    if(workerThread.isRunning()){
        workerThread.quit();
        workerThread.wait();
    }
}

// ===================================================================
// === Implementação da Classe Worker
// ===================================================================

TemperatureWorker::TemperatureWorker() {
    // Construtor
}

void TemperatureWorker::process() {
    // Este método é executado na nova thread
    m_process = new QProcess(this);

    connect(m_process, &QProcess::finished, this, &TemperatureWorker::onProcessFinished);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &TemperatureWorker::readTemperature);
    m_timer->start(1000);
    readTemperature(); // Leitura inicial
}

void TemperatureWorker::readTemperature() {
    // Verifica se um processo já está em execução
    if (m_process->state() == QProcess::NotRunning) {
        QString appDir = QCoreApplication::applicationDirPath();
        QString program = appDir + "/TempReader.exe";

        // CORREÇÃO: Define o diretório de trabalho para que o .exe encontre a .dll
        m_process->setWorkingDirectory(appDir);
        m_process->start(program, QStringList());
    }
}

void TemperatureWorker::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    QByteArray stdOut = m_process->readAllStandardOutput();
    QByteArray stdErr = m_process->readAllStandardError();

    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        QString resultStr = QString::fromLatin1(stdOut).trimmed();
        resultStr.replace(',', '.');

        // Analisa a string para obter os três valores
        double cpuTemp = -2.0, mbTemp = -2.0, gpuTemp = -2.0;
        QStringList parts = resultStr.split(';', Qt::SkipEmptyParts);

        for (const QString &part : parts) {
            QStringList pair = part.split(':');
            if (pair.length() == 2) {
                bool ok;
                double temp = pair[1].toDouble(&ok);
                if (ok) {
                    if (pair[0] == "CPU") cpuTemp = temp;
                    else if (pair[0] == "MOTHERBOARD") mbTemp = temp;
                    else if (pair[0] == "GPU") gpuTemp = temp;
                }
            }
        }
        emit temperatureUpdated(cpuTemp, mbTemp, gpuTemp);

    } else {
        // O programa auxiliar falhou ao executar
        qDebug() << "Helper process failed to run. Error:" << stdErr;
        emit temperatureUpdated(-1.0, -1.0, -1.0); // Código de erro "falha na execução"
    }
}
