#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "cputemperature.h"
#include "splashscreen.h"
#include <QLabel>
#include <QFontDatabase>
#include <QDebug>
#include <QPalette>
#include <QIcon>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle("LAG Zero | Monitor");
    this->setWindowIcon(QIcon(":/images/logo.png"));

    QFontDatabase::addApplicationFont(":/fonts/Audiowide-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Inter-VariableFont_opsz,wght.ttf");

    this->setStyleSheet("QMainWindow { background-color: #05070d; }");

    // --- Configuração da tela de splash ---
    // A tela de splash é um widget separado que se mostra e depois se fecha
    // automaticamente após um tempo, revelando a janela principal.
    SplashScreen *splash = new SplashScreen();
    splash->show();
    QTimer::singleShot(2000, splash, &SplashScreen::close);
    QTimer::singleShot(2000, this, &MainWindow::show);
    QTimer::singleShot(2000, splash, &SplashScreen::deleteLater);

    // 1. Cria o widget central principal e um layout horizontal para ele
    QWidget* centralWidget = new QWidget(this);
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    setCentralWidget(centralWidget);

    // 2. Cria o painel de navegação (lado esquerdo)
    QVBoxLayout* navLayout = new QVBoxLayout();
    navLayout->setSpacing(10);
    navLayout->setContentsMargins(10, 10, 10, 10);
    navLayout->setAlignment(Qt::AlignTop);

    // Cria e estiliza os botões de navegação principais
    QPushButton* overviewBtn = new QPushButton("Visão Geral", this);
    QPushButton* tempBtn = new QPushButton("Temperatura", this);
    QPushButton* settingsBtn = new QPushButton("Configurações", this);

    // Cria e estiliza os botões para as sub-abas de temperatura
    QPushButton* cpuBtn = new QPushButton("CPU", this);
    QPushButton* gpuBtn = new QPushButton("GPU", this);
    QPushButton* motherboardBtn = new QPushButton("Placa-mãe", this);
    QPushButton* storageBtn = new QPushButton("Armazenamento", this);

    // Estilo dos botões
    QString buttonStyle = "QPushButton {"
                          "    background-color: #1e1e1e;"
                          "    color: #fff;"
                          "    border: 1px solid #333;"
                          "    border-radius: 5px;"
                          "    padding: 10px;"
                          "    font-family: 'Inter';"
                          "    font-weight: bold;"
                          "}"
                          "QPushButton:hover {"
                          "    background-color: #2a2a2a;"
                          "}"
                          "QPushButton:pressed {"
                          "    background-color: #05070d;"
                          "}";
    overviewBtn->setStyleSheet(buttonStyle);
    tempBtn->setStyleSheet(buttonStyle);
    settingsBtn->setStyleSheet(buttonStyle);

    navLayout->addWidget(overviewBtn);
    navLayout->addWidget(tempBtn);
    navLayout->addWidget(settingsBtn);

    // Adiciona o layout de navegação ao layout principal
    mainLayout->addLayout(navLayout);

    // 3. Cria o QStackedWidget principal
    m_mainStackedWidget = new QStackedWidget(this);

    // 4. Cria as páginas principais
    m_overviewPage = new QWidget(this);
    m_temperaturePage = new QWidget(this);
    m_settingsPage = new QWidget(this);

    // --- Configuração da página "Visão Geral" ---
    QVBoxLayout* overviewLayout = new QVBoxLayout(m_overviewPage);
    overviewLayout->addWidget(new QLabel("Visão Geral do sistema", this));
    overviewLayout->addStretch();

    // --- Configuração da página "Temperatura" com suas sub-abas ---
    QVBoxLayout* tempPageLayout = new QVBoxLayout(m_temperaturePage);

    QHBoxLayout* tempNavLayout = new QHBoxLayout();
    tempNavLayout->setAlignment(Qt::AlignLeft);
    tempNavLayout->addWidget(cpuBtn);
    tempNavLayout->addWidget(gpuBtn);
    tempNavLayout->addWidget(motherboardBtn);
    tempNavLayout->addWidget(storageBtn);

    tempPageLayout->addLayout(tempNavLayout);

    m_tempSubStackedWidget = new QStackedWidget(this);

    // Cria as páginas internas da aba de Temperatura
    m_cpuPage = new QWidget(this);
    m_gpuPage = new QWidget(this);
    m_motherboardPage = new QWidget(this);
    m_storagePage = new QWidget(this);

    // Adiciona o conteúdo para as páginas de temperatura
    QVBoxLayout* cpuLayout = new QVBoxLayout(m_cpuPage);
    ui->tempLabel = new QLabel("CPU: -- °C", this);
    cpuLayout->addWidget(ui->tempLabel);
    cpuLayout->addStretch();

    QVBoxLayout* gpuLayout = new QVBoxLayout(m_gpuPage);
    ui->gpuTempLabel = new QLabel("GPU: -- °C", this);
    gpuLayout->addWidget(ui->gpuTempLabel);
    gpuLayout->addStretch();

    QVBoxLayout* motherboardLayout = new QVBoxLayout(m_motherboardPage);
    ui->motherboardTempLabel = new QLabel("Placa-mãe: -- °C", this);
    motherboardLayout->addWidget(ui->motherboardTempLabel);
    motherboardLayout->addStretch();

    QVBoxLayout* storageLayout = new QVBoxLayout(m_storagePage);
    ui->storageLayout = new QVBoxLayout();
    storageLayout->addLayout(ui->storageLayout);
    storageLayout->addStretch();

    // Adiciona as sub-páginas ao QStackedWidget de temperatura
    m_tempSubStackedWidget->addWidget(m_cpuPage);
    m_tempSubStackedWidget->addWidget(m_gpuPage);
    m_tempSubStackedWidget->addWidget(m_motherboardPage);
    m_tempSubStackedWidget->addWidget(m_storagePage);

    tempPageLayout->addWidget(m_tempSubStackedWidget);

    // --- Configuração da página "Configurações" ---
    QVBoxLayout* settingsLayout = new QVBoxLayout(m_settingsPage);
    settingsLayout->addWidget(new QLabel("Conteúdo das Configurações...", this));
    settingsLayout->addStretch();

    // 5. Adiciona as páginas principais ao QStackedWidget principal
    m_mainStackedWidget->addWidget(m_overviewPage);
    m_mainStackedWidget->addWidget(m_temperaturePage);
    m_mainStackedWidget->addWidget(m_settingsPage);

    // 6. Adiciona o QStackedWidget principal ao layout principal
    mainLayout->addWidget(m_mainStackedWidget);

    // 7. Conecta os botões principais aos QStackedWidget para mudar a página
    connect(overviewBtn, &QPushButton::clicked, m_mainStackedWidget, [=]() { m_mainStackedWidget->setCurrentWidget(m_overviewPage); });
    connect(tempBtn, &QPushButton::clicked, m_mainStackedWidget, [=]() { m_mainStackedWidget->setCurrentWidget(m_temperaturePage); });
    connect(settingsBtn, &QPushButton::clicked, m_mainStackedWidget, [=]() { m_mainStackedWidget->setCurrentWidget(m_settingsPage); });

    // 8. Conecta os botões das sub-abas de temperatura para mudar a página interna
    connect(cpuBtn, &QPushButton::clicked, m_tempSubStackedWidget, [=]() { m_tempSubStackedWidget->setCurrentWidget(m_cpuPage); });
    connect(gpuBtn, &QPushButton::clicked, m_tempSubStackedWidget, [=]() { m_tempSubStackedWidget->setCurrentWidget(m_gpuPage); });
    connect(motherboardBtn, &QPushButton::clicked, m_tempSubStackedWidget, [=]() { m_tempSubStackedWidget->setCurrentWidget(m_motherboardPage); });
    connect(storageBtn, &QPushButton::clicked, m_tempSubStackedWidget, [=]() { m_tempSubStackedWidget->setCurrentWidget(m_storagePage); });

    // Inicia o leitor de temperatura
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

// A função getTemperatureColor não é mais necessária, mas a mantive como um exemplo
// para o caso de você querer voltar atrás. Ela não será chamada.
QColor MainWindow::getTemperatureColor(double temperature) {
    // Escala de temperatura: 20°C a 90°C
    double normalizedTemp = qMax(0.0, qMin(1.0, (temperature - 20.0) / (90.0 - 20.0)));

    // Cores de gradiente (azul para verde, verde para vermelho)
    QColor blue(0, 209, 255);   // #00d1ff
    QColor green(52, 255, 126);  // #34ff7e
    QColor red(231, 76, 60);    // #e74c3c

    if (normalizedTemp < 0.5) {
        return QColor(blue.red() + (green.red() - blue.red()) * (normalizedTemp * 2),
                      blue.green() + (green.green() - blue.green()) * (normalizedTemp * 2),
                      blue.blue() + (green.blue() - blue.blue()) * (normalizedTemp * 2));
    } else {
        return QColor(green.red() + (red.red() - green.red()) * ((normalizedTemp - 0.5) * 2),
                      green.green() + (red.green() - green.green()) * ((normalizedTemp - 0.5) * 2),
                      green.blue() + (red.blue() - green.blue()) * ((normalizedTemp - 0.5) * 2));
    }
}

void MainWindow::setLabelText(QLabel* label, const QString& prefix, double temperature)
{
    if (!label) return;

    // Estilo base do rótulo
    QString styleSheet = "QLabel {"
                         "    font-size: 24px;"
                         "    font-family: 'Inter';"
                         "    font-weight: bold;"
                         "    text-shadow: 2px 2px 4px #000000;"
                         "}";

    // Se a temperatura for válida, aplica a cor branca
    if (temperature >= 0) {
        styleSheet += QString("QLabel { color: #bdbdbd; }");
        label->setText(prefix + QString::number(temperature, 'f', 1) + " °C");
    } else {
        styleSheet += QString("QLabel { color: #ecf0f1; }");
        label->setText(prefix + "Não encontrado");
    }

    label->setStyleSheet(styleSheet);
}

void MainWindow::onTemperaturesUpdated(const QMap<QString, double> &temps)
{
    setLabelText(ui->tempLabel, "CPU: ", temps.value("CPU", -1.0));
    setLabelText(ui->motherboardTempLabel, "Placa-mãe: ", temps.value("MOTHERBOARD", -1.0));
    setLabelText(ui->gpuTempLabel, "GPU: ", temps.value("GPU", -1.0));

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
            setLabelText(m_storageLabels.value(key), driveName + ": ", it.value());
        }
    }
}

// Implementação do slot para lidar com os cliques dos botões.
// Mesmo que a conexão seja feita com uma lambda, a declaração do slot ainda é necessária.
void MainWindow::onNavigationButtonClicked()
{
    // A lógica para mudar a página já está na conexão via lambda
    // nos botões, então esta função pode ficar vazia.
    // Ela só precisa existir para o linker não reclamar.
}
