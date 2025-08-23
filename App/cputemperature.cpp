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
    m_timer->start(2000); // Atualiza a cada 2 segundos
    readTemperature(); // Leitura inicial
}

void TemperatureWorker::readTemperature() {
    // Verifica se um processo já está em execução
    if (m_process->state() == QProcess::NotRunning) {
        // O caminho para o executável é relativo ao executável principal do seu App
        QString program = QCoreApplication::applicationDirPath() + "/TempReader.exe";
        m_process->start(program, QStringList());
    }
}

void TemperatureWorker::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    QByteArray stdOut = m_process->readAllStandardOutput();
    QByteArray stdErr = m_process->readAllStandardError();

    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        QString tempStr = QString::fromLatin1(stdOut).trimmed();

        // --- CORREÇÃO: Substitui a vírgula pelo ponto ---
        tempStr.replace(',', '.');

        bool ok;
        double temperature = tempStr.toDouble(&ok);

        if (ok && !tempStr.isEmpty()) {
            emit temperatureUpdated(temperature);
        } else {
            // O programa rodou mas não retornou um número válido
            emit temperatureUpdated(-2.0); // Código de erro "não encontrado"
        }
    } else {
        // O programa auxiliar falhou ao executar
        qDebug() << "Helper process failed to run. Error:" << stdErr;
        emit temperatureUpdated(-1.0); // Código de erro "falha na execução"
    }
}
