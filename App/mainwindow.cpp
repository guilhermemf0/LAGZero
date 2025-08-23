#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "cputemperature.h" // Inclui o header da classe de temperatura
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_tempReader = new CpuTemperature(this);
    connect(m_tempReader, &CpuTemperature::temperaturesUpdated, this, &MainWindow::onTemperaturesUpdated);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Função auxiliar para formatar o texto de um label
void setLabelText(QLabel* label, const QString& prefix, double temperature)
{
    if (!label) return;
    if (temperature >= 0) {
        label->setText(prefix + QString::number(temperature, 'f', 1) + " °C");
    } else {
        label->setText(prefix + "Não encontrado");
    }
}

void MainWindow::onTemperaturesUpdated(const QMap<QString, double> &temps)
{
    // Atualiza os labels fixos
    setLabelText(ui->tempLabel, "CPU: ", temps.value("CPU", -1.0));
    setLabelText(ui->motherboardTempLabel, "Placa-mãe: ", temps.value("MOTHERBOARD", -1.0));
    setLabelText(ui->gpuTempLabel, "GPU: ", temps.value("GPU", -1.0));

    // Atualiza os labels dinâmicos dos discos
    for (auto it = temps.constBegin(); it != temps.constEnd(); ++it)
    {
        const QString &key = it.key();
        if (key.startsWith("STORAGE_"))
        {
            // Extrai o nome do disco do identificador
            QStringList keyParts = key.split('_');
            QString driveName = (keyParts.size() > 2) ? keyParts.at(2) : key;

            // Se o label para este disco ainda não existe, cria-o
            if (!m_storageLabels.contains(key))
            {
                QLabel *newLabel = new QLabel(this);
                ui->storageLayout->addWidget(newLabel); // Adiciona ao nosso layout vertical
                m_storageLabels.insert(key, newLabel);
            }

            // Atualiza o texto do label
            setLabelText(m_storageLabels.value(key), driveName + ": ", it.value());
        }
    }
}
