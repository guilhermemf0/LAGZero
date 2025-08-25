#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "hardwaremonitor.h"
#include "fpsmonitor.h"
#include <QFontDatabase>
#include <QIcon>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QStyle>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->statusbar->hide();
    this->setWindowTitle("LAG Zero | Monitor");
    this->setWindowIcon(QIcon(":/images/logo.png"));
    this->setMinimumSize(800, 600);

    QFontDatabase::addApplicationFont(":/fonts/Audiowide-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Inter-VariableFont_opsz,wght.ttf");

    QString globalStyleSheet = R"(
        #centralwidget { background-color: #05070d; font-family: 'Inter'; }
        #navPanel { background-color: #10121a; border-right: 1px solid #222; }
        #navPanel QPushButton {
            background-color: transparent; color: #aeb9d6; border: none;
            padding: 15px; text-align: left; font-size: 14px; font-weight: bold;
            border-left: 3px solid transparent;
        }
        #navPanel QPushButton:hover { background-color: #1c1e28; }
        #navPanel QPushButton[selected="true"] {
            color: #ffffff; background-color: #05070d; border-left-color: #0085ff;
        }
        #settingsButton {
            text-align: center; font-size: 16px; padding: 0; margin: 10px;
            border-radius: 5px; min-height: 28px; max-height: 28px;
            min-width: 28px; max-width: 28px; border: 1px solid #222;
        }
        #settingsButton[selected="true"] {
            background-color: #05070d; border: 1px solid #222;
            border-left-color: #222 !important;
        }
        #contentPanel { background-color: #05070d; padding: 20px; }
        #tempNavPanel { border-bottom: 1px solid #222; }
        #tempNavPanel QPushButton {
            background-color: transparent; color: #aeb9d6; border: none;
            padding: 12px 20px; font-size: 14px; font-weight: bold;
            border-bottom: 3px solid transparent;
        }
        #tempNavPanel QPushButton:hover { color: #ffffff; background-color: #1c1e28; }
        #tempNavPanel QPushButton[selected="true"] {
            color: #ffffff; border-bottom-color: #0085ff;
        }
        QLabel[objectName="tempTitleLabel"] { color: #aeb9d6; font-size: 16px; }
        QLabel[objectName="tempValueLabel"] {
            color: #ffffff; font-size: 16px; font-weight: bold;
            min-width: 70px; text-align: right;
        }
    )";
    this->setStyleSheet(globalStyleSheet);

    // --- ESTRUTURA DA UI CORRIGIDA ---
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Painel de Navegação Esquerdo
    QFrame* navPanel = new QFrame();
    navPanel->setObjectName("navPanel");
    navPanel->setFixedWidth(200);
    QVBoxLayout* navLayout = new QVBoxLayout(navPanel);
    navLayout->setContentsMargins(0, 10, 0, 0);
    navLayout->setSpacing(5);

    QPushButton* overviewBtn = new QPushButton("Visão Geral", navPanel);
    QPushButton* tempBtn = new QPushButton("Temperaturas", navPanel);
    QPushButton* settingsBtn = new QPushButton("⚙", navPanel);
    settingsBtn->setObjectName("settingsButton");

    m_navButtons << overviewBtn << tempBtn << settingsBtn;
    for(auto btn : m_navButtons) connect(btn, &QPushButton::clicked, this, &MainWindow::onNavigationButtonClicked);

    navLayout->addWidget(overviewBtn);
    navLayout->addWidget(tempBtn);
    navLayout->addStretch();
    navLayout->addWidget(settingsBtn);

    // Painel de Conteúdo Direito
    QFrame* contentPanel = new QFrame();
    contentPanel->setObjectName("contentPanel");
    QVBoxLayout* contentLayout = new QVBoxLayout(contentPanel);
    m_mainStackedWidget = new QStackedWidget();
    contentLayout->addWidget(m_mainStackedWidget);

    mainLayout->addWidget(navPanel);
    mainLayout->addWidget(contentPanel, 1);

    // --- PÁGINA 1: Visão Geral ---
    QWidget* overviewPage = new QWidget();
    QVBoxLayout* overviewLayout = new QVBoxLayout(overviewPage);
    overviewLayout->setAlignment(Qt::AlignTop);
    overviewLayout->setContentsMargins(0, 10, 0, 0);
    overviewLayout->addWidget(createTemperatureRow("Taxa de Quadros (FPS):", m_fpsTitleLabel, m_fpsValueLabel));
    m_mainStackedWidget->addWidget(overviewPage);

    // --- PÁGINA 2: Temperaturas ---
    QWidget* tempPage = new QWidget();
    QVBoxLayout* tempPageLayout = new QVBoxLayout(tempPage);
    tempPageLayout->setSpacing(20);
    tempPageLayout->setContentsMargins(0, 0, 0, 0);

    QFrame* tempNavPanel = new QFrame();
    tempNavPanel->setObjectName("tempNavPanel");
    QHBoxLayout* tempNavLayout = new QHBoxLayout(tempNavPanel);
    tempNavLayout->setSpacing(10);
    tempNavLayout->setContentsMargins(0, 0, 0, 0);

    QPushButton* cpuTempBtn = new QPushButton("CPU", tempNavPanel);
    QPushButton* gpuTempBtn = new QPushButton("GPU", tempNavPanel);
    QPushButton* mbTempBtn = new QPushButton("Placa-mãe", tempNavPanel);
    QPushButton* storageTempBtn = new QPushButton("Armazenamento", tempNavPanel);
    m_tempNavButtons << cpuTempBtn << gpuTempBtn << mbTempBtn << storageTempBtn;
    for (QPushButton* btn : m_tempNavButtons) {
        tempNavLayout->addWidget(btn);
        connect(btn, &QPushButton::clicked, this, &MainWindow::onTempNavigationButtonClicked);
    }
    tempNavLayout->addStretch();

    m_tempStackedWidget = new QStackedWidget();
    tempPageLayout->addWidget(tempNavPanel);
    tempPageLayout->addWidget(m_tempStackedWidget, 1);
    m_mainStackedWidget->addWidget(tempPage);

    // Sub-páginas de temperatura
    QWidget* cpuPage = new QWidget();
    QVBoxLayout* cpuPageLayout = new QVBoxLayout(cpuPage);
    cpuPageLayout->setAlignment(Qt::AlignTop);
    cpuPageLayout->setContentsMargins(0, 10, 0, 0);
    cpuPageLayout->addWidget(createTemperatureRow("Temperatura da CPU:", m_cpuTitleLabel, m_cpuTempValueLabel));
    m_tempStackedWidget->addWidget(cpuPage);

    QWidget* gpuPage = new QWidget();
    QVBoxLayout* gpuPageLayout = new QVBoxLayout(gpuPage);
    gpuPageLayout->setAlignment(Qt::AlignTop);
    gpuPageLayout->setContentsMargins(0, 10, 0, 0);
    gpuPageLayout->addWidget(createTemperatureRow("Temperatura da GPU:", m_gpuTitleLabel, m_gpuTempValueLabel));
    m_tempStackedWidget->addWidget(gpuPage);

    QWidget* mbPage = new QWidget();
    QVBoxLayout* mbPageLayout = new QVBoxLayout(mbPage);
    mbPageLayout->setAlignment(Qt::AlignTop);
    mbPageLayout->setContentsMargins(0, 10, 0, 0);
    mbPageLayout->addWidget(createTemperatureRow("Temperatura da Placa-mãe:", m_mbTitleLabel, m_mbTempValueLabel));
    m_tempStackedWidget->addWidget(mbPage);

    QWidget* storagePage = new QWidget();
    m_storagePageLayout = new QVBoxLayout(storagePage);
    m_storagePageLayout->setSpacing(10);
    m_storagePageLayout->setAlignment(Qt::AlignTop);
    m_storagePageLayout->setContentsMargins(0, 10, 0, 0);
    m_tempStackedWidget->addWidget(storagePage);

    // --- PÁGINA 3: Configurações ---
    m_mainStackedWidget->addWidget(new QLabel("Configurações (Em breve)"));

    // --- Estado Inicial ---
    overviewBtn->click();
    cpuTempBtn->click();

    // --- Instancia e conecta os monitores ---
    m_hardwareMonitor = new HardwareMonitor(this);
    connect(m_hardwareMonitor, &HardwareMonitor::hardwareUpdated, this, &MainWindow::onHardwareUpdated);

    m_fpsMonitor = new FpsMonitor(this);
    connect(m_fpsMonitor, &FpsMonitor::fpsUpdated, this, &MainWindow::onFpsUpdated);
}

MainWindow::~MainWindow() { delete ui; }

QWidget* MainWindow::createTemperatureRow(const QString &title, QLabel* &titleLabel, QLabel* &valueLabel)
{
    QWidget* rowWidget = new QWidget();
    QHBoxLayout* rowLayout = new QHBoxLayout(rowWidget);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    titleLabel = new QLabel(title, rowWidget);
    titleLabel->setObjectName("tempTitleLabel");
    valueLabel = new QLabel("--", rowWidget);
    valueLabel->setObjectName("tempValueLabel");
    valueLabel->setAlignment(Qt::AlignRight);
    rowLayout->addWidget(titleLabel);
    rowLayout->addStretch();
    rowLayout->addWidget(valueLabel);
    rowWidget->setMaximumHeight(30);
    return rowWidget;
}

void MainWindow::onHardwareUpdated(const QMap<QString, HardwareInfo> &deviceInfos)
{
    auto updateRow = [](QLabel* titleLabel, QLabel* valueLabel, const QString& newTitle, double temperature) {
        if (titleLabel && valueLabel) {
            titleLabel->setText(newTitle);
            if (temperature >= 0) {
                valueLabel->setText(QString::number(temperature, 'f', 1) + " °C");
            } else {
                valueLabel->setText("N/D");
            }
        }
    };

    HardwareInfo cpuInfo = deviceInfos.value("CPU");
    QString cpuTitle = cpuInfo.name != "N/D" ? "Temperatura da " + cpuInfo.name + ":" : "Temperatura da CPU:";
    updateRow(m_cpuTitleLabel, m_cpuTempValueLabel, cpuTitle, cpuInfo.temperature);

    HardwareInfo gpuInfo = deviceInfos.value("GPU");
    QString gpuTitle = gpuInfo.name != "N/D" ? "Temperatura da " + gpuInfo.name + ":" : "Temperatura da GPU:";
    updateRow(m_gpuTitleLabel, m_gpuTempValueLabel, gpuTitle, gpuInfo.temperature);

    HardwareInfo mbInfo = deviceInfos.value("MOTHERBOARD");
    QString mbTitle = mbInfo.name != "N/D" ? "Temperatura da " + mbInfo.name + ":" : "Temperatura da Placa-mãe:";
    updateRow(m_mbTitleLabel, m_mbTempValueLabel, mbTitle, mbInfo.temperature);

    for (auto it = deviceInfos.constBegin(); it != deviceInfos.constEnd(); ++it) {
        if (it.key().startsWith("STORAGE_")) {
            const HardwareInfo& info = it.value();
            if (!m_storageTitleLabels.contains(info.name)) {
                QLabel *newTitleLabel = nullptr, *newValueLabel = nullptr;
                m_storagePageLayout->addWidget(createTemperatureRow("", newTitleLabel, newValueLabel));
                m_storageTitleLabels.insert(info.name, newTitleLabel);
                m_storageValueLabels.insert(info.name, newValueLabel);
            }
            QString storageTitle = "Temperatura do " + info.name + " (" + info.driveType + "):";
            updateRow(m_storageTitleLabels.value(info.name), m_storageValueLabels.value(info.name), storageTitle, info.temperature);
        }
    }
}

void MainWindow::onFpsUpdated(int fps, const QString& appName)
{
    if (m_fpsTitleLabel && m_fpsValueLabel) {
        if (fps > 0 && !appName.isEmpty()) {
            m_fpsTitleLabel->setText("Taxa de Quadros (" + appName + "):");
            m_fpsValueLabel->setText(QString::number(fps));
        } else {
            m_fpsTitleLabel->setText("Taxa de Quadros (FPS):");
            m_fpsValueLabel->setText("N/A");
        }
    }
}

void MainWindow::onNavigationButtonClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button) return;
    int index = m_navButtons.indexOf(button);
    if (index != -1) {
        m_mainStackedWidget->setCurrentIndex(index);
        updateButtonStyles(button, m_navButtons);
    }
}

void MainWindow::onTempNavigationButtonClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button) return;
    int index = m_tempNavButtons.indexOf(button);
    if (index != -1) {
        m_tempStackedWidget->setCurrentIndex(index);
        updateButtonStyles(button, m_tempNavButtons);
    }
}

void MainWindow::updateButtonStyles(QPushButton *activeButton, QList<QPushButton*> &buttonGroup)
{
    for (QPushButton *btn : buttonGroup) {
        btn->setProperty("selected", (btn == activeButton));
        btn->style()->unpolish(btn);
        btn->style()->polish(btn);
    }
}
