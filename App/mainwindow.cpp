#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QString>
#include <QLabel> // Necessário incluir para usar QLabel

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_cpuTempReader = new CpuTemperature(this);
    connect(m_cpuTempReader, &CpuTemperature::temperatureUpdated, this, &MainWindow::onTemperatureUpdated);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Função auxiliar para formatar o texto
void setLabelText(QLabel* label, const QString& prefix, double temperature)
{
    if (!label) return; // Proteção para caso o label não exista

    if (temperature >= 0) {
        // Usando QString::number para evitar erros de compilação
        label->setText(prefix + QString::number(temperature, 'f', 1) + " °C");
    } else if (temperature == -2.0 || temperature == -1.0) { // Trata ambos os erros
        label->setText(prefix + "Não encontrado");
    }
}

void MainWindow::onTemperatureUpdated(double cpu, double motherboard, double gpu)
{
    setLabelText(ui->tempLabel, "CPU: ", cpu);
    setLabelText(ui->motherboardTempLabel, "Placa-mãe: ", motherboard);
    setLabelText(ui->gpuTempLabel, "GPU: ", gpu);
}
