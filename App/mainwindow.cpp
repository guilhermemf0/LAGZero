#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QString>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Cria uma instância do nosso leitor de temperatura
    m_cpuTempReader = new CpuTemperature(this);

    // Conecta o sinal da classe CpuTemperature ao nosso slot
    connect(m_cpuTempReader, &CpuTemperature::temperatureUpdated, this, &MainWindow::onTemperatureUpdated);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onTemperatureUpdated(double temperature)
{
    if (temperature >= 0) {
        // Formata a string para exibir com uma casa decimal e o símbolo de grau
        ui->tempLabel->setText(QString("Temperatura da CPU: %1 °C").arg(temperature, 0, 'f', 1));
    } else if (temperature == -2.0) {
        // Código de erro para sensor não encontrado
        ui->tempLabel->setText("Sensor não encontrado via WMI");
    }
    else {
        // Outros erros de leitura
        ui->tempLabel->setText("Erro na conexão com WMI");
    }
}
