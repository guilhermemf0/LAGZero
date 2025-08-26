#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "hardwaremonitor.h"
#include "fpsmonitor.h"
#include "particleswidget.h"
#include <QFontDatabase>
#include <QIcon>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QStyle>
#include <QDesktopServices>
#include <QUrl>
#include <QSettings>
#include <QCheckBox>
#include <QGridLayout>
#include <QtSvg/QSvgRenderer>
#include <QPainter>
#include <QPixmap>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->statusbar->hide();
    this->setWindowTitle("LAG Zero | Monitor");
    this->setWindowIcon(QIcon(":/images/logo.png"));
    this->setMinimumSize(850, 650);

    QFontDatabase::addApplicationFont(":/fonts/Audiowide-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Inter-VariableFont_opsz,wght.ttf");

    QString globalStyleSheet = R"(
        #centralContainer { background-color: #05070d; }
        #mainUiContainer { font-family: 'Inter'; }
        #navPanel {
            background-color: rgba(16, 18, 26, 0.95);
            border-right: 1px solid #222;
        }
        #navPanel QPushButton {
            background-color: transparent; color: #aeb9d6; border: none;
            padding: 15px; text-align: left; font-size: 14px; font-weight: bold;
            border-left: 3px solid transparent;
        }
        #navPanel QPushButton:hover { background-color: #1c1e28; }
        #navPanel QPushButton[selected="true"] {
            color: #ffffff; background-color: rgba(5, 7, 13, 0.8); border-left-color: #0085ff;
        }
        #settingsButton {
            background-color: transparent;
            border-radius: 22px;
            min-height: 44px; max-height: 44px;
            min-width: 44px; max-width: 44px;
            border: 1px solid #222;
            padding: 0; /* CORREÇÃO: Remove o padding para centralizar o ícone */
        }
        #settingsButton:hover { background-color: #1c1e28; }
        #contentPanel {
             background-color: rgba(5, 7, 13, 0.8);
             padding: 20px;
        }
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
        QLabel, QCheckBox { color: #aeb9d6; font-size: 14px; background-color: transparent; }
        QLabel[objectName="tempTitleLabel"] { font-size: 16px; }
        QLabel[objectName="tempValueLabel"] {
            color: #ffffff; font-size: 16px; font-weight: bold;
        }
        /* --- NOVO ESTILO PARA O CARTÃO MINIMALISTA --- */
        #rtssStatusCard {
            background-color: rgba(40, 20, 20, 0.5); /* Tom avermelhado sutil */
            border: 1px solid #ff5555;
            border-radius: 8px;
            max-height: 60px; /* Altura fixa para ser compacto */
        }
        #rtssTitleLabel {
            font-size: 16px;
            font-weight: bold;
            color: #ffffff;
        }
        #downloadRtssButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0085ff, stop:1 #00d1ff);
            color: white;
            font-weight: bold;
            font-size: 14px;
            padding: 10px 20px;
            border: none;
            border-radius: 19px; /* Metade da altura (38px) */
        }
        #downloadRtssButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0070e0, stop:1 #00b8e0);
        }
        QCheckBox::indicator {
            width: 16px; height: 16px; border: 1px solid #0085ff;
            border-radius: 3px; background-color: #1c1e28;
        }
        QCheckBox::indicator:checked { background-color: #0085ff; border: 1px solid #0085ff; }
        QCheckBox::indicator:hover { border-color: #00d1ff; }
    )";
    this->setStyleSheet(globalStyleSheet);

    QWidget* centralContainer = new QWidget(this);
    centralContainer->setObjectName("centralContainer");
    setCentralWidget(centralContainer);

    QGridLayout* layeredLayout = new QGridLayout(centralContainer);
    layeredLayout->setContentsMargins(0, 0, 0, 0);

    m_particlesWidget = new ParticlesWidget(centralContainer);
    layeredLayout->addWidget(m_particlesWidget, 0, 0);

    QWidget* mainUiContainer = new QWidget(centralContainer);
    mainUiContainer->setObjectName("mainUiContainer");
    mainUiContainer->setAttribute(Qt::WA_TranslucentBackground);
    QHBoxLayout* mainLayout = new QHBoxLayout(mainUiContainer);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    layeredLayout->addWidget(mainUiContainer, 0, 0);

    QFrame* navPanel = new QFrame();
    navPanel->setObjectName("navPanel");
    navPanel->setFixedWidth(200);
    QVBoxLayout* navLayout = new QVBoxLayout(navPanel);
    navLayout->setContentsMargins(10, 10, 10, 10);
    navLayout->setSpacing(5);

    QPushButton* overviewBtn = new QPushButton("Visão Geral", navPanel);
    QPushButton* tempBtn = new QPushButton("Temperaturas", navPanel);
    m_settingsButton = new QPushButton(navPanel);
    m_settingsButton->setObjectName("settingsButton");
    m_settingsButton->setCursor(Qt::PointingHandCursor);
    updateSettingsButtonIcon(false);

    m_navButtons << overviewBtn << tempBtn;
    for(auto btn : m_navButtons) connect(btn, &QPushButton::clicked, this, &MainWindow::onNavigationButtonClicked);
    connect(m_settingsButton, &QPushButton::clicked, this, &MainWindow::onSettingsButtonClicked);

    navLayout->addWidget(overviewBtn);
    navLayout->addWidget(tempBtn);
    navLayout->addStretch();
    navLayout->addWidget(m_settingsButton, 0, Qt::AlignCenter);

    QFrame* contentPanel = new QFrame();
    contentPanel->setObjectName("contentPanel");
    QVBoxLayout* contentLayout = new QVBoxLayout(contentPanel);
    m_mainStackedWidget = new QStackedWidget();
    contentLayout->addWidget(m_mainStackedWidget);

    mainLayout->addWidget(navPanel);
    mainLayout->addWidget(contentPanel, 1);

    QWidget* overviewPage = new QWidget();
    overviewPage->setAttribute(Qt::WA_TranslucentBackground);
    QVBoxLayout* overviewLayout = new QVBoxLayout(overviewPage);
    overviewLayout->setAlignment(Qt::AlignTop);
    overviewLayout->setContentsMargins(0, 10, 0, 0);
    overviewLayout->setSpacing(15);

    // --- CRIAÇÃO DO NOVO CARTÃO MINIMALISTA ---
    m_rtssStatusCard = new QFrame(overviewPage);
    m_rtssStatusCard->setObjectName("rtssStatusCard");

    QHBoxLayout *rtssLayout = new QHBoxLayout(m_rtssStatusCard);
    rtssLayout->setContentsMargins(15, 10, 15, 10);
    rtssLayout->setSpacing(15);

    m_rtssStatusIcon = new QLabel();
    m_rtssStatusIcon->setFixedSize(24, 24);
    rtssLayout->addWidget(m_rtssStatusIcon);

    m_rtssTitleLabel = new QLabel("RTSS não detectado", m_rtssStatusCard);
    m_rtssTitleLabel->setObjectName("rtssTitleLabel");
    rtssLayout->addWidget(m_rtssTitleLabel, 1); // O '1' faz ele ocupar o espaço restante

    m_downloadRtssButton = new QPushButton("Download", m_rtssStatusCard);
    m_downloadRtssButton->setObjectName("downloadRtssButton");
    m_downloadRtssButton->setCursor(Qt::PointingHandCursor);
    m_downloadRtssButton->setMinimumHeight(38);
    connect(m_downloadRtssButton, &QPushButton::clicked, this, &MainWindow::onDownloadRtssClicked);
    rtssLayout->addWidget(m_downloadRtssButton);

    overviewLayout->addWidget(m_rtssStatusCard);
    overviewLayout->addWidget(createDataRow(":/icons/fps.svg", "Taxa de Quadros:", m_fpsTitleLabel, m_fpsValueLabel));
    overviewLayout->addStretch();
    m_mainStackedWidget->addWidget(overviewPage);

    QWidget* tempPage = new QWidget();
    tempPage->setAttribute(Qt::WA_TranslucentBackground);
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

    auto createTempPage = [&](const QString& iconPath, const QString& title, QLabel*& titleLabel, QLabel*& valueLabel) {
        QWidget* page = new QWidget();
        QVBoxLayout* layout = new QVBoxLayout(page);
        layout->setAlignment(Qt::AlignTop);
        layout->setContentsMargins(0, 20, 0, 0);
        layout->setSpacing(15);
        layout->addWidget(createDataRow(iconPath, title, titleLabel, valueLabel));
        layout->addStretch();
        return page;
    };

    m_tempStackedWidget->addWidget(createTempPage(":/icons/cpu.svg", "Temperatura da CPU:", m_cpuTitleLabel, m_cpuTempValueLabel));
    m_tempStackedWidget->addWidget(createTempPage(":/icons/gpu.svg", "Temperatura da GPU:", m_gpuTitleLabel, m_gpuTempValueLabel));
    m_tempStackedWidget->addWidget(createTempPage(":/icons/motherboard.svg", "Temperatura da Placa-mãe:", m_mbTitleLabel, m_mbTempValueLabel));

    QWidget* storagePage = new QWidget();
    m_storagePageLayout = new QVBoxLayout(storagePage);
    m_storagePageLayout->setSpacing(15);
    m_storagePageLayout->setAlignment(Qt::AlignTop);
    m_storagePageLayout->setContentsMargins(0, 20, 0, 0);
    m_tempStackedWidget->addWidget(storagePage);

    setupSettingsPage();

    overviewBtn->click();
    cpuTempBtn->click();

    m_hardwareMonitor = new HardwareMonitor(this);
    connect(m_hardwareMonitor, &HardwareMonitor::hardwareUpdated, this, &MainWindow::onHardwareUpdated);

    m_fpsMonitor = new FpsMonitor(this);
    connect(m_fpsMonitor, &FpsMonitor::fpsUpdated, this, &MainWindow::onFpsUpdated);
    connect(m_fpsMonitor, &FpsMonitor::rtssStatusUpdated, this, &MainWindow::onRtssStatusUpdated);

    QSettings settings("LAGZero", "MonitorApp");
    bool particlesEnabled = settings.value("particles/enabled", true).toBool();
    m_enableParticlesCheckBox->setChecked(particlesEnabled);
    onParticlesEnabledChanged(particlesEnabled ? Qt::Checked : Qt::Unchecked);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::setupSettingsPage()
{
    QWidget* settingsPage = new QWidget();
    QVBoxLayout* settingsLayout = new QVBoxLayout(settingsPage);
    settingsLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    settingsLayout->setContentsMargins(20, 20, 20, 20);
    settingsLayout->setSpacing(15);

    QLabel* settingsTitle = new QLabel("Configurações", settingsPage);
    settingsTitle->setStyleSheet("font-family: 'Audiowide'; font-size: 24px; color: #ffffff; margin-bottom: 10px; background-color: transparent;");
    settingsLayout->addWidget(settingsTitle);

    m_enableParticlesCheckBox = new QCheckBox("Ativar partículas de fundo", settingsPage);
    connect(m_enableParticlesCheckBox, &QCheckBox::checkStateChanged, this, [this](Qt::CheckState state){
        this->onParticlesEnabledChanged(state);
    });
    settingsLayout->addWidget(m_enableParticlesCheckBox);

    settingsLayout->addStretch();

    m_mainStackedWidget->addWidget(settingsPage);
}

QWidget* MainWindow::createDataRow(const QString &iconPath, const QString &title, QLabel* &titleLabel, QLabel* &valueLabel)
{
    QWidget* rowWidget = new QWidget();
    QHBoxLayout* rowLayout = new QHBoxLayout(rowWidget);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(12);

    QString svgContent;
    if (iconPath == ":/icons/cpu.svg") svgContent = R"(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#aeb9d6" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="9" y="9" width="6" height="6" rx="1"></rect><rect x="5" y="5" width="14" height="14" rx="2"></rect><path d="M5 12H2m20 0h-3M12 5V2m0 20v-3"></path></svg>)";
    else if (iconPath == ":/icons/gpu.svg") svgContent = R"(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#aeb9d6" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="3" y="3" width="18" height="18" rx="2"></rect><circle cx="9" cy="9" r="2"></circle><path d="M14.5 9.5 21 3m-4 11-4.5 4.5M3 21l8-8"></path></svg>)";
    else if (iconPath == ":/icons/motherboard.svg") svgContent = R"(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#aeb9d6" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="2" y="2" width="20" height="20" rx="2.18" ry="2.18"></rect><line x1="7" y1="12" x2="7" y2="12.01"></line><line x1="12" y1="7" x2="12" y2="7.01"></line><line x1="12" y1="12" x2="12" y2="12.01"></line><line x1="12" y1="17" x2="12" y2="17.01"></line><line x1="17" y1="12" x2="17" y2="12.01"></line></svg>)";
    else if (iconPath == ":/icons/storage.svg") svgContent = R"(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#aeb9d6" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><ellipse cx="12" cy="5" rx="9" ry="3"></ellipse><path d="M21 12c0 1.66-4 3-9 3s-9-1.34-9-3"></path><path d="M3 5v14c0 1.66 4 3 9 3s9-1.34 9-3V5"></path></svg>)";
    else if (iconPath == ":/icons/fps.svg") svgContent = R"(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#aeb9d6" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="m12 14 4-4"></path><path d="M3.34 19a10 10 0 1 1 17.32 0"></path></svg>)";

    if (!svgContent.isEmpty()) {
        QSvgRenderer renderer;
        renderer.load(svgContent.toUtf8());
        QPixmap pixmap(QSize(24, 24));
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        renderer.render(&painter);
        QLabel* iconLabel = new QLabel();
        iconLabel->setFixedSize(QSize(24, 24));
        iconLabel->setPixmap(pixmap);
        rowLayout->addWidget(iconLabel);
    }

    titleLabel = new QLabel(title, rowWidget);
    titleLabel->setObjectName("tempTitleLabel");
    valueLabel = new QLabel("--", rowWidget);
    valueLabel->setObjectName("tempValueLabel");
    valueLabel->setAlignment(Qt::AlignLeft);
    rowLayout->addWidget(titleLabel);
    rowLayout->addWidget(valueLabel);
    rowLayout->addStretch();
    rowWidget->setMaximumHeight(40);
    return rowWidget;
}

void MainWindow::onHardwareUpdated(const QMap<QString, HardwareInfo> &deviceInfos)
{
    auto updateRow = [](QLabel* titleLabel, QLabel* valueLabel, const QString& newTitle, double temperature) {
        if (titleLabel && valueLabel) {
            titleLabel->setText(newTitle);
            if (temperature >= 0) {
                valueLabel->setText(" " + QString::number(temperature, 'f', 1) + " °C");
            } else {
                valueLabel->setText(" N/D");
            }
        }
    };

    HardwareInfo cpuInfo = deviceInfos.value("CPU");
    QString cpuTitle = cpuInfo.name != "N/D" ? cpuInfo.name + ":" : "Temperatura da CPU:";
    updateRow(m_cpuTitleLabel, m_cpuTempValueLabel, cpuTitle, cpuInfo.temperature);

    HardwareInfo gpuInfo = deviceInfos.value("GPU");
    QString gpuTitle = gpuInfo.name != "N/D" ? gpuInfo.name + ":" : "Temperatura da GPU:";
    updateRow(m_gpuTitleLabel, m_gpuTempValueLabel, gpuTitle, gpuInfo.temperature);

    HardwareInfo mbInfo = deviceInfos.value("MOTHERBOARD");
    QString mbTitle = mbInfo.name != "N/D" ? mbInfo.name + ":" : "Temperatura da Placa-mãe:";
    updateRow(m_mbTitleLabel, m_mbTempValueLabel, mbTitle, mbInfo.temperature);

    for (auto it = deviceInfos.constBegin(); it != deviceInfos.constEnd(); ++it) {
        if (it.key().startsWith("STORAGE_")) {
            const HardwareInfo& info = it.value();
            if (!m_storageTitleLabels.contains(info.name)) {
                QLabel *newTitleLabel = nullptr, *newValueLabel = nullptr;
                m_storagePageLayout->addWidget(createDataRow(":/icons/storage.svg", "", newTitleLabel, newValueLabel));
                m_storageTitleLabels.insert(info.name, newTitleLabel);
                m_storageValueLabels.insert(info.name, newValueLabel);
            }
            QString storageTitle = info.name + " (" + info.driveType + "):";
            updateRow(m_storageTitleLabels.value(info.name), m_storageValueLabels.value(info.name), storageTitle, info.temperature);
        }
    }
}

void MainWindow::onFpsUpdated(int fps, const QString& appName)
{
    if (m_fpsTitleLabel && m_fpsValueLabel) {
        if (fps > 0 && !appName.isEmpty()) {
            m_fpsTitleLabel->setText("Taxa de Quadros (" + appName + "):");
            m_fpsValueLabel->setText(" " + QString::number(fps));
        } else {
            m_fpsTitleLabel->setText("Taxa de Quadros:");
            if (m_rtssStatusCard->isVisible()) {
                m_fpsValueLabel->setText(" N/A");
            } else {
                m_fpsValueLabel->setText(" Aguardando jogo...");
            }
        }
    }
}

void MainWindow::onRtssStatusUpdated(bool found, const QString& installPath)
{
    m_rtssStatusCard->setVisible(!found);

    if (!found) {
        QString iconSvg = R"(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#ff5555" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="10"></circle><line x1="12" y1="8" x2="12" y2="12"></line><line x1="12" y1="16" x2="12.01" y2="16"></line></svg>)";
        QSvgRenderer renderer;
        renderer.load(iconSvg.toUtf8());
        QPixmap pixmap(m_rtssStatusIcon->size());
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        renderer.render(&painter);
        m_rtssStatusIcon->setPixmap(pixmap);
    }
    onFpsUpdated(0, "");
    Q_UNUSED(installPath);
}

void MainWindow::onDownloadRtssClicked()
{
    QUrl rtssDownloadUrl("https://www.guru3d.com/getdownload/2c1b2414f56a6594ffef91236a87c0e976d52e0519b1373846bab016c2f20c7c4d6ce7dfe19a0bc843da8d448bbb670058b0c9ee9a26f5cf49bc39c97da070e6eb314629af3da2d24ab0413917f73b946419b5af447da45cefb517a0840ad3003abff4f9d5fe7828bbbb910ee270b20632035fba6a450da22325b6bc5b6ecf760e598e0a09bb891387012d7e49a92b4a8f1c86af94bc77d60086a77bc5d35333a4a994caecfec48d7150f6815800664825e01e9acc364dec6f");
    QDesktopServices::openUrl(rtssDownloadUrl);
}

void MainWindow::onParticlesEnabledChanged(Qt::CheckState state)
{
    QSettings settings("LAGZero", "MonitorApp");
    bool enabled = (state == Qt::Checked);
    settings.setValue("particles/enabled", enabled);

    if (enabled) {
        m_particlesWidget->show();
        m_particlesWidget->startAnimation();
    } else {
        m_particlesWidget->hide();
        m_particlesWidget->stopAnimation();
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
        updateSettingsButtonIcon(false);
    }
}

void MainWindow::onSettingsButtonClicked()
{
    m_mainStackedWidget->setCurrentIndex(m_navButtons.count());
    updateButtonStyles(nullptr, m_navButtons);
    updateSettingsButtonIcon(true);
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

void MainWindow::updateSettingsButtonIcon(bool selected)
{
    QString color = selected ? "#00d1ff" : "#aeb9d6";
    QString svgContent = QString(R"(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="%1" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="3"></circle><path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1 0 2.83 2 2 0 0 1-2.83 0l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-2 2 2 2 0 0 1-2-2v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83 0 2 2 0 0 1 0-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1-2-2 2 2 0 0 1 2-2h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 0-2.83 2 2 0 0 1 2.83 0l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 2-2 2 2 0 0 1 2 2v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 0 2 2 0 0 1 0 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 2 2 2 2 0 0 1-2 2h-.09a1.65 1.65 0 0 0-1.51 1z"></path></svg>)").arg(color);

    QSvgRenderer renderer;
    renderer.load(svgContent.toUtf8());
    QPixmap pixmap(m_settingsButton->iconSize());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    renderer.render(&painter);
    m_settingsButton->setIcon(QIcon(pixmap));

    m_settingsButton->setProperty("selected", selected);
    m_settingsButton->style()->unpolish(m_settingsButton);
    m_settingsButton->style()->polish(m_settingsButton);
}
