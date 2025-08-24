#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "cputemperature.h"
#include <QLabel>
#include <QFontDatabase>
#include <QDebug>
#include <QPalette>
#include <QIcon>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle("LAG Zero | Monitor");
    this->setWindowIcon(QIcon(":/images/logo.png"));

    QFontDatabase::addApplicationFont(":/fonts/Audiowide-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Inter-VariableFont_opsz,wght.ttf");

    // Estilo simplificado para uma janela sem abas
    this->setStyleSheet(
        "QMainWindow { background-color: #05070d; }"
        "QLabel {"
        "   color: #ecf0f1;"
        "   font-family: 'Audiowide';"
        "   font-size: 14pt;"
        "   padding-left: 20px;"
        "   padding-top: 5px;"
        "}"
        );

    m_tempReader = new CpuTemperature(this);
    connect(m_tempReader, &CpuTemperature::temperaturesUpdated, this, &MainWindow::onTemperaturesUpdated);
}

MainWindow::~MainWindow()
{
    delete ui;
}

QColor MainWindow::getCpuColor(double temperature)
{
    if (temperature > 95.0) return QColor("#e74c3c");
    if (temperature > 85.0) return QColor("#f39c12");
    if (temperature >= 75.0) return QColor("#3498db");
    return QColor("#00d1ff");
}

void MainWindow::setLabelText(QLabel* label, const QString& prefix, double temperature, bool isCpu)
{
    if (!label) return;
    QPalette palette = label->palette();
    if (isCpu) {
        palette.setColor(QPalette::WindowText, getCpuColor(temperature));
    } else {
        palette.setColor(QPalette::WindowText, QColor("#ecf0f1"));
    }
    label->setPalette(palette);

    if (temperature >= 0) {
        label->setText(prefix + QString::number(temperature, 'f', 1) + " °C");
    } else {
        label->setText(prefix + "Não encontrado");
    }
}

void MainWindow::onTemperaturesUpdated(const QMap<QString, double> &temps)
{
    setLabelText(ui->tempLabel, "CPU: ", temps.value("CPU", -1.0), true);
    setLabelText(ui->motherboardTempLabel, "Placa-mãe: ", temps.value("MOTHERBOARD", -1.0), false);
    setLabelText(ui->gpuTempLabel, "GPU: ", temps.value("GPU", -1.0), false);

    for (auto it = temps.constBegin(); it != temps.constEnd(); ++it)
    {
        const QString &key = it.key();
        if (key.startsWith("STORAGE_"))
        {
            QStringList keyParts = key.split('_');
            QString driveName = "Disco";
            if (keyParts.size() > 2) {
                keyParts.removeFirst();
                keyParts.removeFirst();
                driveName = keyParts.join(' ');
            }
            if (!m_storageLabels.contains(key))
            {
                QLabel *newLabel = new QLabel(this);
                ui->storageLayout->addWidget(newLabel);
                m_storageLabels.insert(key, newLabel);
            }
            setLabelText(m_storageLabels.value(key), driveName + ": ", it.value(), false);
        }
    }
}
