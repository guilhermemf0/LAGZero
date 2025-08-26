#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "fpsmonitor.h"
#include "gamecoverwidget.h"
#include <QFontDatabase>
#include <QIcon>
#include <QStyle>
#include <QDesktopServices>
#include <QSettings>
#include <QGridLayout>
#include <QtSvg/QSvgRenderer>
#include <QPainter>
#include <QPixmap>
#include <QScrollArea>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QFileInfo>
#include <QStandardPaths>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_hardwareMonitor = new HardwareMonitor(this);
    m_fpsMonitor = new FpsMonitor(this);
    m_apiManager = new ApiManager(this);

    setupUi();
    setupConnections();

    populateRecentGames();
    setActiveGameView(false); // Começa com a visão de "espera"

    m_navButtons.first()->click();
    m_tempNavButtons.first()->click();

    QSettings settings("LAGZero", "MonitorApp");
    bool particlesEnabled = settings.value("particles/enabled", true).toBool();
    m_enableParticlesCheckBox->setChecked(particlesEnabled);
    onParticlesEnabledChanged(particlesEnabled ? Qt::Checked : Qt::Unchecked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Configura a estrutura geral da UI
void MainWindow::setupUi()
{
    ui->statusbar->hide();
    this->setWindowTitle("LAG Zero | Monitor");
    this->setWindowIcon(QIcon(":/images/logo.png"));
    this->setMinimumSize(900, 700);

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
            padding: 0;
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
        #rtssStatusCard {
            background-color: rgba(40, 20, 20, 0.5);
            border: 1px solid #ff5555;
            border-radius: 8px;
            max-height: 60px;
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
            border-radius: 19px;
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
        #activeGameCoverLabel {
            border-radius: 12px;
            background-color: #111;
        }
        #activeGameNameLabel {
            font-family: 'Audiowide';
            font-size: 32px;
            color: #ffffff;
            font-weight: normal;
        }
        #activeGameFpsLabel {
            font-size: 28px;
            font-weight: bold;
            color: #00d1ff;
        }
        #waitingForGameLabel {
            font-size: 24px;
            color: #aeb9d6;
            font-family: 'Audiowide';
        }
        #recentGamesTitle {
            font-size: 20px;
            font-weight: bold;
            color: #ffffff;
            margin-top: 15px;
            margin-bottom: 5px;
        }
        QScrollArea {
            background: transparent;
            border: none;
        }
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

    setupOverviewPage();
    setupTempPage();
    setupSettingsPage();
}

void MainWindow::setupConnections()
{
    connect(m_hardwareMonitor, &HardwareMonitor::hardwareUpdated, this, &MainWindow::onHardwareUpdated);

    connect(m_fpsMonitor, &FpsMonitor::rtssStatusUpdated, this, &MainWindow::onRtssStatusUpdated);
    connect(m_fpsMonitor, &FpsMonitor::gameSessionStarted, this, &MainWindow::onGameSessionStarted);
    connect(m_fpsMonitor, &FpsMonitor::gameSessionEnded, this, &MainWindow::onGameSessionEnded);
    connect(m_fpsMonitor, &FpsMonitor::activeGameFpsUpdate, this, &MainWindow::onActiveGameFpsUpdate);

    connect(m_apiManager, &ApiManager::searchFinished, this, &MainWindow::onApiSearchFinished);
    connect(m_apiManager, &ApiManager::imageDownloaded, this, &MainWindow::onImageDownloaded);

    for(auto btn : m_navButtons) connect(btn, &QPushButton::clicked, this, &MainWindow::onNavigationButtonClicked);
    for (QPushButton* btn : m_tempNavButtons) connect(btn, &QPushButton::clicked, this, &MainWindow::onTempNavigationButtonClicked);
    connect(m_settingsButton, &QPushButton::clicked, this, &MainWindow::onSettingsButtonClicked);

    connect(m_downloadRtssButton, &QPushButton::clicked, this, &MainWindow::onDownloadRtssClicked);
    connect(m_enableParticlesCheckBox, &QCheckBox::checkStateChanged, this, &MainWindow::onParticlesEnabledChanged);
}

void MainWindow::setupOverviewPage()
{
    QWidget* overviewPage = new QWidget();
    auto* pageLayout = new QVBoxLayout(overviewPage);
    pageLayout->setContentsMargins(10, 10, 10, 10);
    pageLayout->setSpacing(15);

    // --- 1. Alerta RTSS (no topo) ---
    m_rtssStatusCard = new QFrame();
    m_rtssStatusCard->setObjectName("rtssStatusCard");
    QHBoxLayout *rtssLayout = new QHBoxLayout(m_rtssStatusCard);
    rtssLayout->setContentsMargins(15, 10, 15, 10);
    rtssLayout->setSpacing(15);
    m_rtssStatusIcon = new QLabel();
    m_rtssStatusIcon->setFixedSize(24, 24);
    rtssLayout->addWidget(m_rtssStatusIcon);
    m_rtssTitleLabel = new QLabel("RTSS não detectado", m_rtssStatusCard);
    m_rtssTitleLabel->setObjectName("rtssTitleLabel");
    rtssLayout->addWidget(m_rtssTitleLabel, 1);
    m_downloadRtssButton = new QPushButton("Download", m_rtssStatusCard);
    m_downloadRtssButton->setObjectName("downloadRtssButton");
    m_downloadRtssButton->setCursor(Qt::PointingHandCursor);
    m_downloadRtssButton->setMinimumHeight(38);
    rtssLayout->addWidget(m_downloadRtssButton);
    pageLayout->addWidget(m_rtssStatusCard);

    // --- 2. Widget para Jogo Ativo (inicialmente oculto) ---
    m_activeGameWidget = new QWidget();
    auto* activeLayout = new QHBoxLayout(m_activeGameWidget);
    activeLayout->setSpacing(20);
    m_activeGameCoverLabel = new QLabel();
    m_activeGameCoverLabel->setObjectName("activeGameCoverLabel");
    m_activeGameCoverLabel->setFixedSize(180, 270);
    m_activeGameCoverLabel->setScaledContents(true);
    auto* infoLayout = new QVBoxLayout();
    m_activeGameNameLabel = new QLabel("Game Name");
    m_activeGameNameLabel->setObjectName("activeGameNameLabel");
    m_activeGameFpsLabel = new QLabel("--- FPS");
    m_activeGameFpsLabel->setObjectName("activeGameFpsLabel");
    infoLayout->addWidget(m_activeGameNameLabel);
    infoLayout->addWidget(m_activeGameFpsLabel);
    infoLayout->addStretch();
    activeLayout->addWidget(m_activeGameCoverLabel);
    activeLayout->addLayout(infoLayout, 1);
    pageLayout->addWidget(m_activeGameWidget);

    // --- 3. Label de "Aguardando" (inicialmente visível) ---
    m_waitingForGameLabel = new QLabel("Aguardando Jogo...");
    m_waitingForGameLabel->setObjectName("waitingForGameLabel");
    m_waitingForGameLabel->setAlignment(Qt::AlignCenter);
    pageLayout->addWidget(m_waitingForGameLabel);
    pageLayout->addStretch(1);

    // --- 4. Seção de Jogos Recentes ---
    auto* recentTitle = new QLabel("Jogos Recentes");
    recentTitle->setObjectName("recentGamesTitle");
    pageLayout->addWidget(recentTitle);

    m_recentGamesScrollArea = new QScrollArea();
    m_recentGamesScrollArea->setWidgetResizable(true);
    m_recentGamesScrollArea->setFixedHeight(240); // Aumentado para acomodar a animação
    m_recentGamesContainer = new QWidget();
    m_recentGamesLayout = new QHBoxLayout(m_recentGamesContainer);
    m_recentGamesLayout->setContentsMargins(10, 0, 10, 0);
    m_recentGamesLayout->setSpacing(15);
    m_recentGamesLayout->addStretch(); // Para alinhar à esquerda
    m_recentGamesScrollArea->setWidget(m_recentGamesContainer);
    pageLayout->addWidget(m_recentGamesScrollArea);

    m_mainStackedWidget->addWidget(overviewPage);
}

void MainWindow::setupTempPage()
{
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
}

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
    settingsLayout->addWidget(m_enableParticlesCheckBox);

    settingsLayout->addStretch();

    m_mainStackedWidget->addWidget(settingsPage);
}

void MainWindow::populateRecentGames()
{
    QLayoutItem* item;
    while ((item = m_recentGamesLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }

    QList<GameData> recentGames = DatabaseManager::instance().getGamesByMostRecent();
    for (const auto& gameData : recentGames) {
        QPixmap cover(gameData.coverPath);
        auto* coverWidget = new GameCoverWidget(gameData.displayName, cover);
        m_recentGamesLayout->insertWidget(m_recentGamesLayout->count() - 1, coverWidget);
    }
    m_recentGamesLayout->addStretch();
}

void MainWindow::setActiveGameView(bool active)
{
    m_activeGameWidget->setVisible(active);
    m_waitingForGameLabel->setVisible(!active);
}

void MainWindow::onGameSessionStarted(const QString& exeName, uint32_t processId)
{
    setActiveGameView(true);
    GameData data = DatabaseManager::instance().getGameData(exeName);

    if (data.id != -1 && !data.displayName.isEmpty()) {
        m_activeGameNameLabel->setText(data.displayName);
        m_activeGameCoverLabel->setPixmap(QPixmap(data.coverPath));
    } else {
        m_activeGameNameLabel->setText(QFileInfo(exeName).baseName());
        m_activeGameCoverLabel->clear();
        m_apiManager->searchGame(exeName);
    }
}

void MainWindow::onGameSessionEnded(uint32_t processId, const QString& exeName, double averageFps)
{
    setActiveGameView(false);
    m_activeGameNameLabel->clear();
    m_activeGameCoverLabel->clear();
    m_activeGameFpsLabel->setText("--- FPS");

    int gameId = DatabaseManager::instance().getGameId(exeName);
    if (gameId != -1) {
        DatabaseManager::instance().addGameSession(gameId, 0, QDateTime::currentSecsSinceEpoch(), averageFps);
        populateRecentGames();
    }
}

void MainWindow::onActiveGameFpsUpdate(uint32_t processId, int currentFps)
{
    if(m_activeGameWidget->isVisible()) {
        m_activeGameFpsLabel->setText(QString::number(currentFps) + " FPS");
    }
}

void MainWindow::onApiSearchFinished(const ApiGameResult& result)
{
    if (result.success) {
        QString coverDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/covers";
        QString coverPath = coverDir + "/" + QFileInfo(result.executableName).baseName() + ".png";

        int gameId = DatabaseManager::instance().getGameId(result.executableName);
        if (gameId == -1) {
            DatabaseManager::instance().addGame(result.executableName, result.name, coverPath);
        } else {
            DatabaseManager::instance().updateGame(gameId, result.name, coverPath);
        }
        m_apiManager->downloadImage(QUrl(result.coverUrl), coverPath);
    } else {
        if (!DatabaseManager::instance().isGameKnown(result.executableName)) {
            DatabaseManager::instance().addGame(result.executableName, QFileInfo(result.executableName).baseName(), "");
            populateRecentGames();
        }
    }
}

void MainWindow::onImageDownloaded(const QString& localPath, const QUrl& originalUrl)
{
    if(m_activeGameWidget->isVisible()) {
        // Extrai o nome do executável do caminho do ficheiro local
        QString baseName = QFileInfo(localPath).baseName();
        GameData data = DatabaseManager::instance().getGameData(baseName + ".exe");
        if(data.id != -1 && m_activeGameNameLabel->text() == baseName) {
            m_activeGameNameLabel->setText(data.displayName);
            m_activeGameCoverLabel->setPixmap(QPixmap(data.coverPath));
        }
    }
    populateRecentGames();
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
    updateRow(m_cpuTitleLabel, m_cpuTempValueLabel, cpuInfo.name != "N/D" ? cpuInfo.name + ":" : "Temperatura da CPU:", cpuInfo.temperature);
    HardwareInfo gpuInfo = deviceInfos.value("GPU");
    updateRow(m_gpuTitleLabel, m_gpuTempValueLabel, gpuInfo.name != "N/D" ? gpuInfo.name + ":" : "Temperatura da GPU:", gpuInfo.temperature);
    HardwareInfo mbInfo = deviceInfos.value("MOTHERBOARD");
    updateRow(m_mbTitleLabel, m_mbTempValueLabel, mbInfo.name != "N/D" ? mbInfo.name + ":" : "Temperatura da Placa-mãe:", mbInfo.temperature);

    for (auto it = deviceInfos.constBegin(); it != deviceInfos.constEnd(); ++it) {
        if (it.key().startsWith("STORAGE_")) {
            const HardwareInfo& info = it.value();
            if (!m_storageTitleLabels.contains(info.name)) {
                QLabel *newTitleLabel = nullptr, *newValueLabel = nullptr;
                m_storagePageLayout->addWidget(createDataRow(":/icons/storage.svg", "", newTitleLabel, newValueLabel));
                m_storageTitleLabels.insert(info.name, newTitleLabel);
                m_storageValueLabels.insert(info.name, newValueLabel);
            }
            updateRow(m_storageTitleLabels.value(info.name), m_storageValueLabels.value(info.name), info.name + " (" + info.driveType + "):", info.temperature);
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
    Q_UNUSED(installPath);
}

void MainWindow::onDownloadRtssClicked()
{
    QUrl rtssDownloadUrl("https://www.guru3d.com/download/rtss-rivatuner-statistics-server-download/");
    QDesktopServices::openUrl(rtssDownloadUrl);
}

void MainWindow::onParticlesEnabledChanged(int state)
{
    bool enabled = (static_cast<Qt::CheckState>(state) == Qt::Checked);
    QSettings settings("LAGZero", "MonitorApp");
    settings.setValue("particles/enabled", enabled);

    m_particlesWidget->setVisible(enabled);
    if (enabled) {
        m_particlesWidget->startAnimation();
    } else {
        m_particlesWidget->stopAnimation();
    }
}

void MainWindow::onNavigationButtonClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if(!button) return;
    int index = m_navButtons.indexOf(button);
    if (index != -1) {
        m_mainStackedWidget->setCurrentIndex(index);
        updateButtonStyles(button, m_navButtons);
        updateSettingsButtonIcon(false);
    }
}

void MainWindow::onSettingsButtonClicked()
{
    m_mainStackedWidget->setCurrentIndex(m_mainStackedWidget->count() - 1);
    updateButtonStyles(nullptr, m_navButtons);
    updateSettingsButtonIcon(true);
}

void MainWindow::onTempNavigationButtonClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if(!button) return;
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
    QPixmap pixmap(m_settingsButton->size());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    renderer.render(&painter);
    m_settingsButton->setIcon(QIcon(pixmap));

    m_settingsButton->setProperty("selected", selected);
    m_settingsButton->style()->unpolish(m_settingsButton);
    m_settingsButton->style()->polish(m_settingsButton);
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
