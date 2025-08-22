#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>
#include <QLabel>
#include <QTimer>
#include <QMessageBox>
#include <QProcess>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_tempLabel = new QLabel("Temperatura do CPU: --", this);
    setCentralWidget(m_tempLabel);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::updateCpuTemperature);
    m_timer->start(3000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateCpuTemperature()
{
    qDebug() << "Coletando dados de temperatura via PowerShell...";

    QProcess *process = new QProcess(this);
    
    // Conecte o sinal finished ao novo slot, e adicione o deleteLater
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &MainWindow::onProcessFinished);
    connect(process, &QProcess::finished, process, &QProcess::deleteLater);

    QString program = "powershell.exe";
    QStringList arguments;
    arguments << "-NoProfile" << "-ExecutionPolicy" << "Bypass" << "-File" << "get_temp.ps1";

    // O comando está correto, o problema é a execução.
    // O PowerShell pode não estar no PATH. Vamos usar o caminho completo.
    // Tente o caminho completo: "C:/Windows/System32/WindowsPowerShell/v1.0/powershell.exe"
    // Ou uma variável de ambiente: %SystemRoot%\\system32\\WindowsPowerShell\\v1.0\\powershell.exe
    // Vamos tentar o caminho mais simples primeiro. Se não funcionar, mudamos para o caminho completo.
    
    process->setWorkingDirectory("bin");
    process->start(program, arguments);
    
    // REMOVIDO: A chamada waitForFinished()
}

void MainWindow::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess *process = qobject_cast<QProcess*>(sender());
    if (!process) return;

    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        QByteArray output = process->readAllStandardOutput();
        QString tempString = QString::fromUtf8(output).trimmed();

        qDebug() << "Saida do PowerShell: " << tempString;

        bool ok;
        double temperature = tempString.toDouble(&ok);
        if (ok) {
            qDebug() << "Temperatura encontrada: " << temperature << "°C";
            m_tempLabel->setText("Temperatura do CPU: " + QString::number(temperature, 'f', 2) + " °C");
            
            if (temperature > 85.0) {
                QMessageBox::warning(this, "Aviso de Temperatura", "Cuidado, o seu PC está sobrecarregado, recomendamos que resfrie o computador.");
            }
        } else {
            qWarning() << "Saída do PowerShell não é um número válido: " << tempString;
            m_tempLabel->setText("Temperatura do CPU: Erro na Leitura");
        }
    } else {
        qWarning() << "O processo PowerShell terminou com erro. Código: " << exitCode;
        qWarning() << "Saída de erro: " << process->readAllStandardError();
        m_tempLabel->setText("Temperatura do CPU: Erro (Execução)");
    }
}