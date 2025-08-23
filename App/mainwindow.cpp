#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "cputemperature.h" // Inclui o header da classe de temperatura
#include <QLabel>
#include <QPalette> // Necessário para alterar cores de forma mais eficiente

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // --- INÍCIO DAS MODIFICAÇÕES DE ESTILO ---

    // Define um título para a janela
    this->setWindowTitle("Monitor de Hardware");

    // Define um estilo geral para a janela (tema escuro)
    this->setStyleSheet(
        "QMainWindow {"
        "   background-color: #16202a;" // Fundo alterado para a nova cor
        "}"
        "QLabel {"
        "   color: #ecf0f1;" // Cor do texto padrão (cinza muito claro)
        "   font-family: 'Segoe UI';" // Fonte mais moderna
        "   font-size: 14pt;" // Tamanho da fonte
        "   font-weight: bold;" // Fonte em negrito
        "}"
    );

    // --- FIM DAS MODIFICAÇÕES DE ESTILO ---

    m_tempReader = new CpuTemperature(this);
    connect(m_tempReader, &CpuTemperature::temperaturesUpdated, this, &MainWindow::onTemperaturesUpdated);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Função auxiliar para formatar o texto de um label
void setLabelText(QLabel* label, const QString& prefix, double temperature, bool isCpu = false)
{
    if (!label) return; // Proteção para caso o label não exista

    // Usa QPalette para alterar apenas a cor do texto, preservando o estilo da fonte
    QPalette palette = label->palette();
    QColor color("#ecf0f1"); // Cor padrão (cinza muito claro)

    // Lógica para mudar a cor do texto da CPU com base em faixas de temperatura
    if (isCpu) {
        if (temperature > 95.0) {
            // Acima de 95 graus: Vermelho
            color = QColor("#e74c3c");
        } else if (temperature > 85.0) {
            // Entre 85 e 95 graus: Laranja
            color = QColor("#f39c12");
        } else if (temperature >= 75.0) {
            // Entre 75 e 85 graus: Azul
            color = QColor("#3498db");
        }
    }

    palette.setColor(QPalette::WindowText, color);
    label->setPalette(palette);

    if (temperature >= 0) {
        label->setText(prefix + QString::number(temperature, 'f', 1) + " °C");
    } else {
        label->setText(prefix + "Não encontrado");
    }
}

void MainWindow::onTemperaturesUpdated(const QMap<QString, double> &temps)
{
    // Atualiza os labels, passando 'true' para a CPU
    setLabelText(ui->tempLabel, "CPU: ", temps.value("CPU", -1.0), true);
    setLabelText(ui->motherboardTempLabel, "Placa-mãe: ", temps.value("MOTHERBOARD", -1.0));
    setLabelText(ui->gpuTempLabel, "GPU: ", temps.value("GPU", -1.0));

    // Atualiza os labels dinâmicos dos discos
    for (auto it = temps.constBegin(); it != temps.constEnd(); ++it)
    {
        const QString &key = it.key();
        if (key.startsWith("STORAGE_"))
        {
            // Extrai o nome do disco do identificador de forma mais robusta
            QStringList keyParts = key.split('_');
            QString driveName = "Disco"; // Nome padrão
            if (keyParts.size() > 2) {
                keyParts.removeFirst(); // Remove "STORAGE"
                keyParts.removeFirst(); // Remove o índice
                driveName = keyParts.join(' '); // Junta o resto do nome
            }

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
