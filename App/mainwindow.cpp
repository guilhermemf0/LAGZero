#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "fpsmonitor.h"
#include "gamecoverwidget.h"
#include "launchermanager.h"
#include "databasemanager.h"
#include "appconstants.h"
#include "steamappcache.h"
#include "summarycardwidget.h"
#include "coverselectiondialog.h"
#include "infocardwidget.h" // Adicionado para evitar erro de tipo incompleto
#include <QFontDatabase>
#include <QIcon>
#include <QStyle>
#include <QCryptographicHash>
#include <QDesktopServices>
#include <QButtonGroup>
#include <QSettings>
#include <QtSvg/QSvgRenderer>
#include <QPainter>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QTextStream>
#include <QButtonGroup>
#include <QGridLayout>
#include <QGroupBox>
#include <QGraphicsDropShadowEffect>
#include <QRegularExpression>
#include <numeric>
#include <algorithm>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QDebug>
#include <QDirIterator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QGroupBox>
#include <QScrollArea>
#include <QApplication>
#include "infocardwidget.h" // Adicionado para evitar erro de tipo incompleto// <--- ADICIONE ESTA LINHA
#include <QFontDatabase>

// Funções auxiliares
QString cleanEmulatorWindowTitle(QString windowTitle) {
    if (windowTitle.contains('|')) {
        windowTitle = windowTitle.section('|', 1).trimmed();
    }
    return windowTitle;
}

static QString getTempColor(double temp, const QString& type)
{
    if (temp < 0) return "#aeb9d6";
    if (type == AppConfig::CPU_KEY) {
        if (temp < 70) return "#81C784"; if (temp < 85) return "#FFD54F"; return "#D32F2F";
    }
    if (type == AppConfig::GPU_KEY) {
        if (temp < 65) return "#81C784"; if (temp < 80) return "#FFD54F"; return "#D32F2F";
    }
    if (type == AppConfig::MB_KEY) {
        if (temp < 55) return "#81C784"; if (temp < 65) return "#FFD54F"; return "#D32F2F";
    }
    if (temp < 50) return "#81C784"; if (temp < 60) return "#FFD54F"; return "#D32F2F";
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_hardwareMonitor = new HardwareMonitor(this);
    m_fpsMonitor = new FpsMonitor(this);
    m_apiManager = new ApiManager(this);
    m_sessionTimer = new QTimer(this);

    SteamAppCache::instance();

    setupUi();
    setupConnections();

    populateRecentGames();
    populateLibrary();
    setActiveGameView(false);
    m_navButtons.first()->click();

    QSettings settings("LAGZero", "MonitorApp");
    m_enableParticlesCheckBox->setChecked(settings.value(AppConfig::SETTING_PARTICLES_ENABLED, true).toBool());
    onParticlesEnabledChanged(m_enableParticlesCheckBox->isChecked() ? Qt::Checked : Qt::Unchecked);
    m_saveReportsCheckBox->setChecked(settings.value(AppConfig::SETTING_REPORTS_ENABLED, true).toBool());
    m_chartDurationComboBox->setCurrentIndex(settings.value("chart/durationIndex", 1).toInt());
    m_reportFormatComboBox->setCurrentIndex(settings.value("reports/formatIndex", 0).toInt());

    m_overlayEnabledCheckBox->blockSignals(true);
    m_overlayEnabledCheckBox->setChecked(settings.value(AppConfig::SETTING_OVERLAY_ENABLED, true).toBool());
    m_overlayEnabledCheckBox->blockSignals(false);

    m_positionSelector->setCurrentPosition(settings.value(AppConfig::SETTING_OVERLAY_POSITION, 0).toInt());

    int savedStyleId = settings.value(AppConfig::SETTING_OVERLAY_STYLE, 0).toInt();
    if (auto* btn = m_styleSelector->buttonGroup()->button(savedStyleId)) {
        btn->setChecked(true);
    }

    m_overlayShowCpuTempCheckBox->blockSignals(true);
    m_overlayShowCpuTempCheckBox->setChecked(settings.value(AppConfig::SETTING_OVERLAY_SHOW_CPU_TEMP, true).toBool());
    m_overlayShowCpuTempCheckBox->blockSignals(false);
    m_overlayShowCpuUsageCheckBox->blockSignals(true);
    m_overlayShowCpuUsageCheckBox->setChecked(settings.value(AppConfig::SETTING_OVERLAY_SHOW_CPU_USAGE, true).toBool());
    m_overlayShowCpuUsageCheckBox->blockSignals(false);
    m_overlayShowCpuCoresCheckBox->blockSignals(true);
    m_overlayShowCpuCoresCheckBox->setChecked(settings.value(AppConfig::SETTING_OVERLAY_SHOW_CPU_CORES, true).toBool());
    m_overlayShowCpuCoresCheckBox->blockSignals(false);
    m_overlayShowGpuTempCheckBox->blockSignals(true);
    m_overlayShowGpuTempCheckBox->setChecked(settings.value(AppConfig::SETTING_OVERLAY_SHOW_GPU_TEMP, true).toBool());
    m_overlayShowGpuTempCheckBox->blockSignals(false);
    m_overlayShowGpuUsageCheckBox->blockSignals(true);
    m_overlayShowGpuUsageCheckBox->setChecked(settings.value(AppConfig::SETTING_OVERLAY_SHOW_GPU_USAGE, true).toBool());
    m_overlayShowGpuUsageCheckBox->blockSignals(false);
    m_overlayShowRamUsageCheckBox->blockSignals(true);
    m_overlayShowRamUsageCheckBox->setChecked(settings.value(AppConfig::SETTING_OVERLAY_SHOW_RAM_USAGE, true).toBool());
    m_overlayShowRamUsageCheckBox->blockSignals(false);
    m_overlayShowMbTempCheckBox->blockSignals(true);
    m_overlayShowMbTempCheckBox->setChecked(settings.value(AppConfig::SETTING_OVERLAY_SHOW_MB_TEMP, true).toBool());
    m_overlayShowMbTempCheckBox->blockSignals(false);
    m_overlayShowStorageTempCheckBox->blockSignals(true);
    m_overlayShowStorageTempCheckBox->setChecked(settings.value(AppConfig::SETTING_OVERLAY_SHOW_STORAGE_TEMP, true).toBool());
    m_overlayShowStorageTempCheckBox->blockSignals(false);
    m_overlayShowAvgFpsCheckBox->blockSignals(true);
    m_overlayShowAvgFpsCheckBox->setChecked(settings.value(AppConfig::SETTING_OVERLAY_SHOW_AVG_FPS, true).toBool());
    m_overlayShowAvgFpsCheckBox->blockSignals(false);
    m_overlayShowMinFpsCheckBox->blockSignals(true);
    m_overlayShowMinFpsCheckBox->setChecked(settings.value(AppConfig::SETTING_OVERLAY_SHOW_MIN_FPS, true).toBool());
    m_overlayShowMinFpsCheckBox->blockSignals(false);
    m_overlayShowMaxFpsCheckBox->blockSignals(true);
    m_overlayShowMaxFpsCheckBox->setChecked(settings.value(AppConfig::SETTING_OVERLAY_SHOW_MAX_FPS, true).toBool());
    m_overlayShowMaxFpsCheckBox->blockSignals(false);

    m_overlayShowCpuPowerCheckBox->blockSignals(true);
    m_overlayShowCpuPowerCheckBox->setChecked(settings.value(AppConfig::SETTING_OVERLAY_SHOW_CPU_POWER, true).toBool());
    m_overlayShowCpuPowerCheckBox->blockSignals(false);
    m_overlayShowCpuClockCheckBox->blockSignals(true);
    m_overlayShowCpuClockCheckBox->setChecked(settings.value(AppConfig::SETTING_OVERLAY_SHOW_CPU_CLOCK, true).toBool());
    m_overlayShowCpuClockCheckBox->blockSignals(false);
    m_overlayShowGpuPowerCheckBox->blockSignals(true);
    m_overlayShowGpuPowerCheckBox->setChecked(settings.value(AppConfig::SETTING_OVERLAY_SHOW_GPU_POWER, true).toBool());
    m_overlayShowGpuPowerCheckBox->blockSignals(false);
    m_overlayShowGpuClockCheckBox->blockSignals(true);
    m_overlayShowGpuClockCheckBox->setChecked(settings.value(AppConfig::SETTING_OVERLAY_SHOW_GPU_CLOCK, true).toBool());
    m_overlayShowGpuClockCheckBox->blockSignals(false);
    m_overlayShowFansCheckBox->blockSignals(true);
    m_overlayShowFansCheckBox->setChecked(settings.value(AppConfig::SETTING_OVERLAY_SHOW_FANS, true).toBool());
    m_overlayShowFansCheckBox->blockSignals(false);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::setupUi()
{
    ui->statusbar->hide();
    this->setWindowTitle("LAG Zero | Monitor");
    this->setWindowIcon(QIcon(":/images/logo.png"));
    this->setMinimumSize(1100, 740);

    QFontDatabase::addApplicationFont(":/fonts/Audiowide-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Inter-VariableFont_opsz,wght.ttf");

    QFile file(":/style.qss");
    file.open(QFile::ReadOnly | QFile::Text);
    QString styleSheet = QLatin1String(file.readAll());
    this->setStyleSheet(styleSheet);
    file.close();

    auto* centralContainer = new QWidget(this);
    centralContainer->setObjectName("centralContainer");
    setCentralWidget(centralContainer);
    auto* layeredLayout = new QGridLayout(centralContainer);
    layeredLayout->setContentsMargins(0,0,0,0);

    m_particlesWidget = new ParticlesWidget(centralContainer);
    auto* mainUiContainer = new QWidget(centralContainer);
    mainUiContainer->setObjectName("mainUiContainer");
    auto* mainLayout = new QHBoxLayout(mainUiContainer);
    mainLayout->setSpacing(0); mainLayout->setContentsMargins(0,0,0,0);
    layeredLayout->addWidget(m_particlesWidget, 0, 0);
    layeredLayout->addWidget(mainUiContainer, 0, 0);

    auto* navPanel = new QFrame(); navPanel->setObjectName("navPanel");
    navPanel->setFixedWidth(220);
    auto* navLayout = new QVBoxLayout(navPanel);
    navLayout->setContentsMargins(0, 15, 0, 15); navLayout->setSpacing(5);

    m_navButtons << new QPushButton(" Visão Geral")
                 << new QPushButton(" Biblioteca")
                 << new QPushButton(" Componentes");

    for(auto btn : m_navButtons) navLayout->addWidget(btn);
    navLayout->addStretch();
    m_settingsButton = new QPushButton(); m_settingsButton->setObjectName("settingsButton");
    m_settingsButton->setCursor(Qt::PointingHandCursor);
    navLayout->addWidget(m_settingsButton); // Remove o Qt::AlignCenter

    auto* contentPanel = new QFrame(); contentPanel->setObjectName("contentPanel");
    auto* contentLayout = new QVBoxLayout(contentPanel);
    m_mainStackedWidget = new QStackedWidget();
    contentLayout->addWidget(m_mainStackedWidget);
    mainLayout->addWidget(navPanel);
    mainLayout->addWidget(contentPanel, 1);

    setupOverviewPage();
    setupLibraryPage();
    setupTempPage();
    setupSettingsPage();
}

void MainWindow::setupFansPage(QVBoxLayout *layout)
{
    // 1. Remove o título (será o título da "caixa")
    // 2. Remove o m_fansScrollArea (agora é redundante)

    // Apenas cria o container (que tem o grid) e o adiciona ao layout
    m_fansContainer = new QWidget();
    // --- MUDANÇA: Damos um ID para o QSS ---
    m_fansContainer->setObjectName("fansBoxContainer");

    m_fansLayout = new QGridLayout(m_fansContainer);
    m_fansLayout->setSpacing(20);
    m_fansLayout->setAlignment(Qt::AlignTop);

    // Adiciona o container (onde o onHardwareUpdated colocará os cards)
    // diretamente ao layout da "caixa" de Fans
    layout->addWidget(m_fansContainer);
}

void MainWindow::setupConnections() {
    connect(m_hardwareMonitor, &HardwareMonitor::hardwareUpdated, this, &MainWindow::onHardwareUpdated);
    connect(m_hardwareMonitor, &HardwareMonitor::hardwareUpdated, m_fpsMonitor, &FpsMonitor::onHardwareUpdated);
    connect(m_fpsMonitor, &FpsMonitor::rtssStatusUpdated, this, &MainWindow::onRtssStatusUpdated);
    connect(m_fpsMonitor, &FpsMonitor::gameSessionStarted, this, &MainWindow::onGameSessionStarted);
    connect(m_fpsMonitor, &FpsMonitor::gameSessionEnded, this, &MainWindow::onGameSessionEnded);
    connect(m_fpsMonitor, &FpsMonitor::activeGameFpsUpdate, this, &MainWindow::onActiveGameFpsUpdate);
    connect(m_apiManager, &ApiManager::searchFinished, this, &MainWindow::onApiSearchFinished);
    connect(m_apiManager, &ApiManager::imageDownloaded, this, &MainWindow::onImageDownloaded);
    for(auto btn : m_navButtons) connect(btn, &QPushButton::clicked, this, &MainWindow::onNavigationButtonClicked);
    for (auto btn : m_tempNavButtons) connect(btn, &QPushButton::clicked, this, &MainWindow::onTempNavigationButtonClicked);
    connect(m_settingsButton, &QPushButton::clicked, this, &MainWindow::onSettingsButtonClicked);
    connect(m_sessionTimer, &QTimer::timeout, this, &MainWindow::updateSessionInfo);
    connect(m_hardwareMonitor, &HardwareMonitor::helperMissing, this, &MainWindow::onHelperMissing);
    connect(m_enableParticlesCheckBox, &QCheckBox::checkStateChanged, this, &MainWindow::onParticlesEnabledChanged);
    connect(m_saveReportsCheckBox, &QCheckBox::checkStateChanged, this, &MainWindow::onSaveReportsChanged);
    connect(m_chartDurationComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onChartDurationChanged);
    connect(m_apiManager, &ApiManager::gridListAvailable, this, &MainWindow::onGridListReady);

    connect(m_overlayEnabledCheckBox, &QCheckBox::checkStateChanged, this, &MainWindow::onOverlaySettingChanged);
    connect(m_positionSelector, &OverlayPositionSelector::positionSelected, this, &MainWindow::onOverlayPositionChanged);
    connect(m_styleSelector->buttonGroup(), &QButtonGroup::idClicked, this, &MainWindow::onOverlayStyleChanged);

    connect(m_overlayShowCpuTempCheckBox, &QCheckBox::checkStateChanged, this, &MainWindow::onOverlaySettingChanged);
    connect(m_overlayShowCpuUsageCheckBox, &QCheckBox::checkStateChanged, this, &MainWindow::onOverlaySettingChanged);
    connect(m_overlayShowCpuCoresCheckBox, &QCheckBox::checkStateChanged, this, &MainWindow::onOverlaySettingChanged);
    connect(m_overlayShowGpuTempCheckBox, &QCheckBox::checkStateChanged, this, &MainWindow::onOverlaySettingChanged);
    connect(m_overlayShowGpuUsageCheckBox, &QCheckBox::checkStateChanged, this, &MainWindow::onOverlaySettingChanged);
    connect(m_overlayShowRamUsageCheckBox, &QCheckBox::checkStateChanged, this, &MainWindow::onOverlaySettingChanged);
    connect(m_overlayShowMbTempCheckBox, &QCheckBox::checkStateChanged, this, &MainWindow::onOverlaySettingChanged);
    connect(m_overlayShowStorageTempCheckBox, &QCheckBox::checkStateChanged, this, &MainWindow::onOverlaySettingChanged);
    connect(m_overlayShowAvgFpsCheckBox, &QCheckBox::checkStateChanged, this, &MainWindow::onOverlaySettingChanged);
    connect(m_overlayShowMinFpsCheckBox, &QCheckBox::checkStateChanged, this, &MainWindow::onOverlaySettingChanged);
    connect(m_overlayShowMaxFpsCheckBox, &QCheckBox::checkStateChanged, this, &MainWindow::onOverlaySettingChanged);

    connect(m_overlayShowCpuPowerCheckBox, &QCheckBox::checkStateChanged, this, &MainWindow::onOverlaySettingChanged);
    connect(m_overlayShowCpuClockCheckBox, &QCheckBox::checkStateChanged, this, &MainWindow::onOverlaySettingChanged);
    connect(m_overlayShowGpuPowerCheckBox, &QCheckBox::checkStateChanged, this, &MainWindow::onOverlaySettingChanged);
    connect(m_overlayShowGpuClockCheckBox, &QCheckBox::checkStateChanged, this, &MainWindow::onOverlaySettingChanged);
    connect(m_overlayShowFansCheckBox, &QCheckBox::checkStateChanged, this, &MainWindow::onOverlaySettingChanged);
}

void MainWindow::setupOverviewPage() {
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setSpacing(20);
    m_mainStackedWidget->addWidget(page);

    m_hardwareStatusCard = new QFrame();
    m_hardwareStatusCard->setObjectName("hardwareStatusCard");
    m_hardwareStatusCard->setProperty("class", "StatusCard");
    auto* hardwareLayout = new QHBoxLayout(m_hardwareStatusCard);
    hardwareLayout->addWidget(new QLabel("O monitoramento de hardware está desativado. O arquivo TempReader.exe não foi encontrado."));
    m_hardwareStatusCard->setVisible(false);
    layout->addWidget(m_hardwareStatusCard);

    m_rtssStatusCard = new QFrame();
    m_rtssStatusCard->setObjectName("rtssStatusCard");
    m_rtssStatusCard->setProperty("class", "StatusCard");
    auto* rtssLayout = new QHBoxLayout(m_rtssStatusCard);
    rtssLayout->addWidget(new QLabel("RTSS não detectado. O monitoramento de FPS está desativado."));
    auto* rtssBtn = new QPushButton("Download"); rtssBtn->setObjectName("downloadRtssButton");
    connect(rtssBtn, &QPushButton::clicked, this, &MainWindow::onDownloadRtssClicked);
    rtssLayout->addWidget(rtssBtn);
    layout->addWidget(m_rtssStatusCard);

    m_activeGameWidget = new QWidget();
    auto* activeLayout = new QHBoxLayout(m_activeGameWidget);
    activeLayout->setSpacing(25);
    m_activeGameCoverLabel = new QLabel();
    m_activeGameCoverLabel->setObjectName("activeGameCoverLabel");
    m_activeGameCoverLabel->setFixedSize(200, 300);
    m_activeGameCoverLabel->setScaledContents(true);
    auto* shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(40); shadow->setColor(QColor(0,0,0,180)); shadow->setOffset(5, 10);
    m_activeGameCoverLabel->setGraphicsEffect(shadow);

    auto* infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(5);
    m_activeGameNameLabel = new QLabel("Game Name"); m_activeGameNameLabel->setObjectName("activeGameNameLabel");
    m_activeGameInfoLabel = new QLabel("..."); m_activeGameInfoLabel->setObjectName("activeGameInfoLabel");

    auto* metricsPanel = new QWidget();
    auto* metricsLayout = new QHBoxLayout(metricsPanel);
    metricsLayout->setContentsMargins(0, 20, 0, 0); metricsLayout->setSpacing(15);
    metricsLayout->addWidget(createMetricCard("FPS Médio", "AVG_FPS"));
    metricsLayout->addWidget(createMetricCard("Pico Temp. CPU", "MAX_CPU"));
    metricsLayout->addWidget(createMetricCard("Pico Temp. GPU", "MAX_GPU"));
    metricsLayout->addStretch();

    infoLayout->addStretch();
    infoLayout->addWidget(m_activeGameNameLabel);
    infoLayout->addWidget(m_activeGameInfoLabel);
    infoLayout->addWidget(metricsPanel);
    infoLayout->addStretch();
    activeLayout->addWidget(m_activeGameCoverLabel);
    activeLayout->addLayout(infoLayout, 1);

    m_waitingForGameLabel = new QLabel("Aguardando Jogo...");
    m_waitingForGameLabel->setObjectName("waitingForGameLabel");
    m_waitingForGameLabel->setAlignment(Qt::AlignCenter);

    auto* recentTitle = new QLabel("Jogados Recentemente"); recentTitle->setProperty("class", "TitleLabel");
    m_recentGamesScrollArea = new QScrollArea();
    // A linha setObjectName foi REMOVIDA
    m_recentGamesScrollArea->setWidgetResizable(true);
    m_recentGamesScrollArea->setFixedHeight(320);
    m_recentGamesContainer = new QWidget();
    m_recentGamesLayout = new QHBoxLayout(m_recentGamesContainer);
    m_recentGamesLayout->setSpacing(20);
    m_recentGamesScrollArea->setWidget(m_recentGamesContainer);

    layout->addWidget(m_activeGameWidget);
    layout->addWidget(m_waitingForGameLabel);
    layout->addStretch();
    layout->addWidget(recentTitle);
    layout->addWidget(m_recentGamesScrollArea);
}

void MainWindow::setupLibraryPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setSpacing(15);
    m_mainStackedWidget->addWidget(page);

    auto* title = new QLabel("Biblioteca de Jogos");
    title->setProperty("class", "TitleLabel");
    layout->addWidget(title);

    m_libraryScrollArea = new QScrollArea();
    // A linha setObjectName foi REMOVIDA
    m_libraryScrollArea->setWidgetResizable(true);

    m_libraryContainer = new QWidget();
    m_libraryLayout = new QGridLayout(m_libraryContainer);
    m_libraryLayout->setSpacing(25);
    m_libraryLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    m_libraryScrollArea->setWidget(m_libraryContainer);
    layout->addWidget(m_libraryScrollArea);
}


void MainWindow::setupTempPage() {
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setSpacing(15);
    m_mainStackedWidget->addWidget(page);

    auto* tempNavPanel = new QFrame(); tempNavPanel->setObjectName("tempNavPanel");
    auto* tempNavLayout = new QHBoxLayout(tempNavPanel);
    tempNavLayout->setContentsMargins(0,0,0,0);

    // --- MUDANÇA 1: Renomeia os botões da sub-navegação ---
    m_tempNavButtons.clear();
    m_tempNavButtons << new QPushButton("CPU") << new QPushButton("GPU") << new QPushButton("Outros");
    for(auto btn : m_tempNavButtons) tempNavLayout->addWidget(btn);
    tempNavLayout->addStretch();

    m_tempStackedWidget = new QStackedWidget();
    layout->addWidget(tempNavPanel);
    layout->addWidget(m_tempStackedWidget, 1);

    // --- PÁGINA 1: CPU (Sem mudança) ---
    auto createCpuPage = [&]() {
        auto* p = new QWidget();
        auto* l = new QVBoxLayout(p);
        l->setSpacing(20); l->setContentsMargins(0, 10, 0, 0);
        m_cpuSummaryCard = new HardwareSummaryCard(AppConfig::ICON_CPU_SVG, "Processador", p);
        m_cpuChart = new PerformanceChartWidget();
        m_cpuChart->setLabels("Temp. CPU (°C)", "Uso CPU (%)");
        l->addWidget(m_cpuSummaryCard);
        l->addWidget(m_cpuChart, 1);
        return p;
    };
    m_tempStackedWidget->addWidget(createCpuPage());

    // --- PÁGINA 2: GPU (Sem mudança) ---
    auto createGpuPage = [&]() {
        auto* p = new QWidget();
        auto* l = new QVBoxLayout(p);
        l->setSpacing(20); l->setContentsMargins(0, 10, 0, 0);
        m_gpuSummaryCard = new HardwareSummaryCard(AppConfig::ICON_GPU_SVG, "Placa de Vídeo", p);
        m_gpuChart = new PerformanceChartWidget();
        m_gpuChart->setLabels("Temp. GPU (°C)", "Uso GPU (%)");
        l->addWidget(m_gpuSummaryCard);
        l->addWidget(m_gpuChart, 1);
        return p;
    };
    m_tempStackedWidget->addWidget(createGpuPage());

    // --- MUDANÇA 2: As páginas "Placa-mãe" e "Armazenamento" são removidas daqui ---
    // (A função createMbPage() é deletada e a criação do m_storageScrollArea é movida)

    // --- PÁGINA 3: "OUTROS" (Página nova e consolidada) ---
    // Esta página conterá Placa-mãe, Armazenamento e Fans.
    auto* othersScrollArea = new QScrollArea();
    othersScrollArea->setObjectName("othersScrollArea"); 
    othersScrollArea->setWidgetResizable(true);

    auto* othersPageWidget = new QWidget();
    auto* othersLayout = new QVBoxLayout(othersPageWidget);
    othersLayout->setSpacing(10); // Espaçamento menor entre os títulos e as caixas
    othersLayout->setAlignment(Qt::AlignTop);

    // --- SEÇÃO 1: PLACA-MÃE (Já é uma "caixa") ---
    auto* mbTitle = new QLabel("Placa-mãe");
    mbTitle->setProperty("class", "SubtitleLabel");

    // 1. Cria a "caixa" para a Placa-mãe
    auto* mbBoxContainer = new QWidget();
    mbBoxContainer->setObjectName("mbBoxContainer"); // <-- Novo ID para o QSS

    // 2. Cria um layout interno para a "caixa"
    auto* mbBoxLayout = new QVBoxLayout(mbBoxContainer);
    mbBoxLayout->setSpacing(20);
    mbBoxLayout->setAlignment(Qt::AlignTop);

    // 3. Cria o card
    m_mbSummaryCard = new InfoCardWidget(AppConfig::ICON_MB_SVG, "Placa-mãe", mbBoxContainer);

    // 4. Adiciona o card DENTRO da "caixa"
    mbBoxLayout->addWidget(m_mbSummaryCard);
    
    // 5. Adiciona o título e a "caixa" ao layout principal
    othersLayout->addWidget(mbTitle);
    othersLayout->addWidget(mbBoxContainer); // <-- Adiciona a "caixa"
    othersLayout->addSpacing(15); // Espaço extra // Espaço extra após a primeira caixa

    // --- SEÇÃO 2: ARMAZENAMENTO (Nova "caixa") ---
    auto* storageTitle = new QLabel("Armazenamento");
    storageTitle->setProperty("class", "SubtitleLabel");
    
    // Removemos o m_storageScrollArea daqui
    m_storageContainer = new QWidget();
    // --- MUDANÇA: Damos um ID para o QSS ---
    m_storageContainer->setObjectName("storageBoxContainer"); 
    
    m_storagePageLayout = new QVBoxLayout(m_storageContainer); // Este layout receberá os cards dos HDs
    m_storagePageLayout->setSpacing(20); 
    m_storagePageLayout->setAlignment(Qt::AlignTop);
    
    othersLayout->addWidget(storageTitle);
    othersLayout->addWidget(m_storageContainer); // Adiciona a "caixa" (container)
    othersLayout->addSpacing(15); // Espaço extra

    // --- SEÇÃO 3: FANS (Nova "caixa") ---
    auto* fansTitle = new QLabel("Fans");
    fansTitle->setProperty("class", "SubtitleLabel");
    
    // Criamos um layout "wrapper" para a caixa das fans
    auto* fansWrapperLayout = new QVBoxLayout();
    // Chamamos nossa função modificada para adicionar o m_fansContainer (com ID) dentro dele
    setupFansPage(fansWrapperLayout); 
    
    othersLayout->addWidget(fansTitle);
    othersLayout->addLayout(fansWrapperLayout); // Adiciona o layout wrapper

    // --- FIM DAS SEÇÕES ---
    othersLayout->addStretch(1); // Empurra tudo para cima
    othersScrollArea->setWidget(othersPageWidget);
    m_tempStackedWidget->addWidget(othersScrollArea);
}

void MainWindow::setupSettingsPage() {
    auto* page = new QWidget();
    auto* pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(0,0,0,0);

    auto* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    pageLayout->addWidget(scrollArea);

    auto* scrollContentWidget = new QWidget();
    scrollArea->setWidget(scrollContentWidget);

    auto* mainLayout = new QVBoxLayout(scrollContentWidget);
    mainLayout->setSpacing(15);
    mainLayout->setAlignment(Qt::AlignTop);

    auto* title = new QLabel("Configurações");
    title->setProperty("class", "TitleLabel");
    mainLayout->addWidget(title);

    auto* appearanceTitle = new QLabel("Aparência e Gráficos");
    appearanceTitle->setProperty("class", "SubtitleLabel");
    mainLayout->addWidget(appearanceTitle);
    auto* appearanceGroup = new QGroupBox();
    auto* appearanceLayout = new QVBoxLayout(appearanceGroup);
    m_enableParticlesCheckBox = new QCheckBox("Ativar efeito de partículas no fundo");
    appearanceLayout->addWidget(m_enableParticlesCheckBox);
    auto* chartLayout = new QHBoxLayout();
    chartLayout->addWidget(new QLabel("Período de tempo do gráfico:"));
    m_chartDurationComboBox = new QComboBox();
    m_chartDurationComboBox->addItem("Últimos 60 segundos", 60);
    m_chartDurationComboBox->addItem("Últimos 2 minutos", 120);
    m_chartDurationComboBox->addItem("Últimos 5 minutos", 300);
    chartLayout->addWidget(m_chartDurationComboBox);
    chartLayout->addStretch();
    appearanceLayout->addLayout(chartLayout);
    mainLayout->addWidget(appearanceGroup);

    auto* overlayTitle = new QLabel("Configuração do Overlay (RTSS)");
    overlayTitle->setProperty("class", "SubtitleLabel");
    mainLayout->addWidget(overlayTitle);
    auto* overlayGroup = new QGroupBox();
    auto* overlayLayout = new QVBoxLayout(overlayGroup);
    overlayLayout->setSpacing(20);
    m_overlayEnabledCheckBox = new QCheckBox("Ativar overlay de performance no jogo");
    overlayLayout->addWidget(m_overlayEnabledCheckBox);

    auto* appearanceOverlayLayout = new QHBoxLayout();
    appearanceOverlayLayout->setSpacing(25);

    auto* posVLayout = new QVBoxLayout();
    posVLayout->setSpacing(8);
    posVLayout->addWidget(new QLabel("Posição do overlay:"));
    m_positionSelector = new OverlayPositionSelector(this);
    posVLayout->addWidget(m_positionSelector, 0, Qt::AlignLeft);
    posVLayout->addStretch();

    auto* styleVLayout = new QVBoxLayout();
    styleVLayout->setSpacing(8);
    styleVLayout->addWidget(new QLabel("Estilo do overlay:"));
    m_styleSelector = new OverlayStyleSelector(this);
    styleVLayout->addWidget(m_styleSelector);
    styleVLayout->addStretch();

    appearanceOverlayLayout->addLayout(posVLayout);
    appearanceOverlayLayout->addLayout(styleVLayout, 1);
    overlayLayout->addLayout(appearanceOverlayLayout);

    auto* contentBox = new QGroupBox("Itens a serem exibidos");
    auto* contentLayout = new QHBoxLayout(contentBox);
    contentLayout->setAlignment(Qt::AlignTop);

    auto* fpsLayout = new QVBoxLayout();
    fpsLayout->setSpacing(8);
    fpsLayout->addWidget(new QLabel("FPS"));
    m_overlayShowAvgFpsCheckBox = new QCheckBox("Exibir FPS Médio");
    m_overlayShowMinFpsCheckBox = new QCheckBox("Exibir FPS Mínimo");
    m_overlayShowMaxFpsCheckBox = new QCheckBox("Exibir FPS Máximo");
    fpsLayout->addWidget(m_overlayShowAvgFpsCheckBox);
    fpsLayout->addWidget(m_overlayShowMinFpsCheckBox);
    fpsLayout->addWidget(m_overlayShowMaxFpsCheckBox);
    fpsLayout->addStretch();

    auto* cpuLayout = new QVBoxLayout();
    cpuLayout->setSpacing(8);
    cpuLayout->addWidget(new QLabel("CPU"));
    m_overlayShowCpuUsageCheckBox = new QCheckBox("Uso de CPU");
    m_overlayShowCpuTempCheckBox = new QCheckBox("Temperatura");
    m_overlayShowCpuPowerCheckBox = new QCheckBox("Potência (Power)");
    m_overlayShowCpuClockCheckBox = new QCheckBox("Clock");
    m_overlayShowCpuCoresCheckBox = new QCheckBox("Temperaturas dos Núcleos");
    cpuLayout->addWidget(m_overlayShowCpuUsageCheckBox);
    cpuLayout->addWidget(m_overlayShowCpuTempCheckBox);
    cpuLayout->addWidget(m_overlayShowCpuPowerCheckBox);
    cpuLayout->addWidget(m_overlayShowCpuClockCheckBox);
    cpuLayout->addWidget(m_overlayShowCpuCoresCheckBox);
    cpuLayout->addStretch();

    auto* gpuLayout = new QVBoxLayout();
    gpuLayout->setSpacing(8);
    gpuLayout->addWidget(new QLabel("GPU"));
    m_overlayShowGpuUsageCheckBox = new QCheckBox("Uso de GPU");
    m_overlayShowGpuTempCheckBox = new QCheckBox("Temperatura");
    m_overlayShowGpuPowerCheckBox = new QCheckBox("Potência (Power)");
    m_overlayShowGpuClockCheckBox = new QCheckBox("Clock");
    gpuLayout->addWidget(m_overlayShowGpuUsageCheckBox);
    gpuLayout->addWidget(m_overlayShowGpuTempCheckBox);
    gpuLayout->addWidget(m_overlayShowGpuPowerCheckBox);
    gpuLayout->addWidget(m_overlayShowGpuClockCheckBox);
    gpuLayout->addStretch();

    auto* otherLayout = new QVBoxLayout();
    otherLayout->setSpacing(8);
    otherLayout->addWidget(new QLabel("Outros"));
    m_overlayShowRamUsageCheckBox = new QCheckBox("Uso de RAM");
    m_overlayShowFansCheckBox = new QCheckBox("Velocidade das Ventoinhas");
    m_overlayShowMbTempCheckBox = new QCheckBox("Temperatura da Placa-mãe");
    m_overlayShowStorageTempCheckBox = new QCheckBox("Temperatura do Armazenamento");
    otherLayout->addWidget(m_overlayShowRamUsageCheckBox);
    otherLayout->addWidget(m_overlayShowFansCheckBox);
    otherLayout->addWidget(m_overlayShowMbTempCheckBox);
    otherLayout->addWidget(m_overlayShowStorageTempCheckBox);
    otherLayout->addStretch();

    contentLayout->addLayout(fpsLayout);
    contentLayout->addLayout(cpuLayout);
    contentLayout->addLayout(gpuLayout);
    contentLayout->addLayout(otherLayout);

    overlayLayout->addWidget(contentBox);
    mainLayout->addWidget(overlayGroup);

    auto* reportsTitle = new QLabel("Relatórios e Dados");
    reportsTitle->setProperty("class", "SubtitleLabel");
    mainLayout->addWidget(reportsTitle);
    auto* reportsGroup = new QGroupBox();
    auto* reportsLayout = new QVBoxLayout(reportsGroup);
    m_saveReportsCheckBox = new QCheckBox("Salvar relatórios de performance da sessão");
    reportsLayout->addWidget(m_saveReportsCheckBox);
    auto* reportFormatLayout = new QHBoxLayout();
    reportFormatLayout->addWidget(new QLabel("Formato do relatório:"));
    m_reportFormatComboBox = new QComboBox();
    m_reportFormatComboBox->addItem("Texto (.txt)");
    m_reportFormatComboBox->addItem("CSV (.csv)");
    reportFormatLayout->addWidget(m_reportFormatComboBox);
    reportFormatLayout->addStretch();
    reportsLayout->addLayout(reportFormatLayout);
    auto* reportsBtn = new QPushButton("Abrir pasta de relatórios");
    reportsBtn->setCursor(Qt::PointingHandCursor);
    reportsBtn->setStyleSheet("background:transparent; color:#0085ff; text-decoration:underline; font-weight: 600;");
    connect(reportsBtn, &QPushButton::clicked, this, &MainWindow::openReportsFolder);
    reportsLayout->addWidget(reportsBtn, 0, Qt::AlignLeft);
    mainLayout->addWidget(reportsGroup);

    auto* dangerZoneTitle = new QLabel("Zona de Perigo");
    dangerZoneTitle->setProperty("class", "SubtitleLabel");
    dangerZoneTitle->setStyleSheet("color: #f87171;");
    mainLayout->addWidget(dangerZoneTitle);
    auto* dangerZoneGroup = new QGroupBox();
    auto* dangerLayout = new QHBoxLayout(dangerZoneGroup);
    auto* clearHistoryBtn = new QPushButton("Limpar Histórico de Jogos");
    clearHistoryBtn->setProperty("class", "dangerButton");
    connect(clearHistoryBtn, &QPushButton::clicked, this, &MainWindow::onClearHistoryClicked);
    dangerLayout->addWidget(clearHistoryBtn);
    auto* clearReportsBtn = new QPushButton("Limpar Todos os Relatórios");
    clearReportsBtn->setProperty("class", "dangerButton");
    connect(clearReportsBtn, &QPushButton::clicked, this, &MainWindow::onClearReportsClicked);
    dangerLayout->addWidget(clearReportsBtn);
    dangerLayout->addStretch();
    mainLayout->addWidget(dangerZoneGroup);

    mainLayout->addStretch();
    m_mainStackedWidget->addWidget(page);
}

void MainWindow::onOverlaySettingChanged()
{
    QSettings settings("LAGZero", "MonitorApp");
    settings.setValue(AppConfig::SETTING_OVERLAY_ENABLED, m_overlayEnabledCheckBox->isChecked());

    settings.setValue(AppConfig::SETTING_OVERLAY_SHOW_CPU_TEMP, m_overlayShowCpuTempCheckBox->isChecked());
    settings.setValue(AppConfig::SETTING_OVERLAY_SHOW_CPU_USAGE, m_overlayShowCpuUsageCheckBox->isChecked());
    settings.setValue(AppConfig::SETTING_OVERLAY_SHOW_CPU_CORES, m_overlayShowCpuCoresCheckBox->isChecked());
    settings.setValue(AppConfig::SETTING_OVERLAY_SHOW_GPU_TEMP, m_overlayShowGpuTempCheckBox->isChecked());
    settings.setValue(AppConfig::SETTING_OVERLAY_SHOW_GPU_USAGE, m_overlayShowGpuUsageCheckBox->isChecked());
    settings.setValue(AppConfig::SETTING_OVERLAY_SHOW_RAM_USAGE, m_overlayShowRamUsageCheckBox->isChecked());
    settings.setValue(AppConfig::SETTING_OVERLAY_SHOW_MB_TEMP, m_overlayShowMbTempCheckBox->isChecked());
    settings.setValue(AppConfig::SETTING_OVERLAY_SHOW_STORAGE_TEMP, m_overlayShowStorageTempCheckBox->isChecked());
    settings.setValue(AppConfig::SETTING_OVERLAY_SHOW_AVG_FPS, m_overlayShowAvgFpsCheckBox->isChecked());
    settings.setValue(AppConfig::SETTING_OVERLAY_SHOW_MIN_FPS, m_overlayShowMinFpsCheckBox->isChecked());
    settings.setValue(AppConfig::SETTING_OVERLAY_SHOW_MAX_FPS, m_overlayShowMaxFpsCheckBox->isChecked());

    settings.setValue(AppConfig::SETTING_OVERLAY_SHOW_CPU_POWER, m_overlayShowCpuPowerCheckBox->isChecked());
    settings.setValue(AppConfig::SETTING_OVERLAY_SHOW_CPU_CLOCK, m_overlayShowCpuClockCheckBox->isChecked());
    settings.setValue(AppConfig::SETTING_OVERLAY_SHOW_GPU_POWER, m_overlayShowGpuPowerCheckBox->isChecked());
    settings.setValue(AppConfig::SETTING_OVERLAY_SHOW_GPU_CLOCK, m_overlayShowGpuClockCheckBox->isChecked());
    settings.setValue(AppConfig::SETTING_OVERLAY_SHOW_FANS, m_overlayShowFansCheckBox->isChecked());
}

void MainWindow::onOverlayPositionChanged(int id)
{
    QSettings settings("LAGZero", "MonitorApp");
    settings.setValue(AppConfig::SETTING_OVERLAY_POSITION, id);
}

void MainWindow::onOverlayStyleChanged(int id)
{
    QSettings settings("LAGZero", "MonitorApp");
    settings.setValue(AppConfig::SETTING_OVERLAY_STYLE, id);
}

void MainWindow::onGameSessionStarted(const QString& exeName, const QString& windowTitle, uint32_t processId)
{
    qDebug() << "[SESSION START] Executable:" << exeName << "| PID:" << processId << "| Window Title:" << windowTitle;

    QString searchName;
    const QStringList emulatorExes = {"dolphin.exe", "cemu.exe", "yuzu.exe", "ryujinx.exe", "pcsx2.exe", "rpcs3.exe"};
    QString currentExeFile = QFileInfo(exeName).fileName().toLower();

    GameData dataFromDb = DatabaseManager::instance().getGameData(exeName);
    if (dataFromDb.id != -1 && !dataFromDb.user_display_name.isEmpty()) {
        searchName = dataFromDb.user_display_name;
    } else if (emulatorExes.contains(currentExeFile)) {
        searchName = cleanEmulatorWindowTitle(windowTitle);
    } else {
        QString accurateGameName = LauncherManager::instance().findGameDisplayName(exeName, processId);
        searchName = accurateGameName.isEmpty() ? (windowTitle.isEmpty() ? currentExeFile : windowTitle) : accurateGameName;
    }

    setActiveGameView(true);
    m_currentSession = CurrentSession();
    m_currentSession.processId = processId;
    m_currentSession.exeName = exeName;

    GameData existingData = DatabaseManager::instance().getGameData(exeName);

    if (existingData.id != -1 && !existingData.displayName.isEmpty() && !existingData.coverPath.isEmpty()) {
        m_currentSession.displayName = existingData.displayName;
        m_currentSession.coverPath = existingData.coverPath;
        m_activeGameNameLabel->setText(existingData.displayName);
        m_activeGameCoverLabel->setPixmap(QPixmap(existingData.coverPath));
    } else {
        m_currentSession.displayName = searchName;
        m_activeGameNameLabel->setText(searchName);
        m_activeGameCoverLabel->clear();
        m_apiManager->findGameInfo(exeName, searchName);
    }

    if (m_cpuChart) m_cpuChart->clearData();
    if (m_gpuChart) m_gpuChart->clearData();

    m_sessionTimer->start(1000);
    m_currentSession.timer.start();
}

void MainWindow::onGameSessionEnded(uint32_t, const QString& exeName, double averageFps)
{
    setActiveGameView(false);
    if (m_saveReportsCheckBox->isChecked()) saveSessionReport();
    m_sessionTimer->stop();

    int gameId = DatabaseManager::instance().getGameId(exeName);
    if (gameId != -1) {
        DatabaseManager::instance().addGameSession(gameId, 0, QDateTime::currentSecsSinceEpoch(), averageFps);
        populateRecentGames();
        populateLibrary();
    }

    if (m_cpuChart) m_cpuChart->clearData();
    if (m_gpuChart) m_gpuChart->clearData();

    m_currentSession = CurrentSession();
}

void MainWindow::onApiSearchFinished(const ApiGameResult& result)
{
    bool isForCurrentSession = (m_currentSession.exeName == result.executableName);
    bool isForCoverChange = (m_coverChangeTargetExe == result.executableName);

    if (!isForCurrentSession && !isForCoverChange) return;

    QString finalDisplayName = result.name.isEmpty() ?
                                   (isForCurrentSession ? m_currentSession.displayName : DatabaseManager::instance().getGameData(result.executableName).displayName)
                                                     : result.name;

    if (isForCurrentSession) {
        m_activeGameNameLabel->setText(finalDisplayName);
        m_currentSession.displayName = finalDisplayName;
    }

    if (result.success) {
        QString coverDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/covers";
        QByteArray exePathBytes = result.executableName.toUtf8();
        QString uniqueId = QCryptographicHash::hash(exePathBytes, QCryptographicHash::Md5).toHex();
        QString coverPath = coverDir + "/" + uniqueId + ".png";

        DatabaseManager::instance().addOrUpdateGame(result.executableName, finalDisplayName, "");
        m_apiManager->downloadImage(QUrl(result.coverUrl), coverPath);
    } else {
        DatabaseManager::instance().addOrUpdateGame(result.executableName, finalDisplayName, "");
        if (isForCoverChange) m_coverChangeTargetExe.clear();
    }
}


void MainWindow::onImageDownloaded(const QString& localPath, const QUrl&)
{
    QString targetExe = m_coverChangeTargetExe.isEmpty() ? m_currentSession.exeName : m_coverChangeTargetExe;
    if (targetExe.isEmpty()) return;

    QString coverFileId = QFileInfo(localPath).baseName();
    QByteArray targetExePathBytes = targetExe.toUtf8();
    QString targetId = QCryptographicHash::hash(targetExePathBytes, QCryptographicHash::Md5).toHex();

    if (coverFileId != targetId) return;

    int gameId = DatabaseManager::instance().getGameId(targetExe);
    if (gameId != -1) {
        DatabaseManager::instance().updateGameCover(gameId, localPath);
        if (m_currentSession.exeName == targetExe) {
            m_currentSession.coverPath = localPath;
            m_activeGameCoverLabel->setPixmap(QPixmap(localPath));
        }
    }

    populateRecentGames();
    populateLibrary();

    if (!m_coverChangeTargetExe.isEmpty()) m_coverChangeTargetExe.clear();
}

void MainWindow::populateRecentGames() {
    QLayoutItem* item;
    while ((item = m_recentGamesLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    for (const auto& gameData : DatabaseManager::instance().getGamesByMostRecent(10)) {
        auto* coverWidget = new GameCoverWidget(gameData.displayName, gameData.executableName, QPixmap(gameData.coverPath));
        connect(coverWidget, &GameCoverWidget::editGameRequested, this, &MainWindow::onManualEditRequested);
        connect(coverWidget, &GameCoverWidget::removeGameRequested, this, &MainWindow::onRemoveGameRequested);
        connect(coverWidget, &GameCoverWidget::changeCoverRequested, this, &MainWindow::triggerCoverChange);
        m_recentGamesLayout->addWidget(coverWidget);
    }
    m_recentGamesLayout->addStretch();
}

void MainWindow::populateLibrary()
{
    QLayoutItem* item;
    while ((item = m_libraryLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    int row = 0, col = 0;
    const int maxCols = 4;

    for (const auto& gameData : DatabaseManager::instance().getAllGames()) {
        auto* coverWidget = new GameCoverWidget(gameData.displayName, gameData.executableName, QPixmap(gameData.coverPath));
        connect(coverWidget, &GameCoverWidget::editGameRequested, this, &MainWindow::onManualEditRequested);
        connect(coverWidget, &GameCoverWidget::removeGameRequested, this, &MainWindow::onRemoveGameRequested);
        connect(coverWidget, &GameCoverWidget::changeCoverRequested, this, &MainWindow::triggerCoverChange);
        m_libraryLayout->addWidget(coverWidget, row, col++);
        if (col >= maxCols) {
            col = 0;
            row++;
        }
    }
}

void MainWindow::triggerCoverChange(const QString& executableName)
{
    m_coverChangeTargetExe = executableName;
    GameData gameData = DatabaseManager::instance().getGameData(executableName);
    if (gameData.id == -1) return;
    m_apiManager->findGameInfo(executableName, gameData.displayName);
}

void MainWindow::onGridListReady(const QString& executableName, const QList<QJsonObject>& gridList)
{
    if (m_coverChangeTargetExe != executableName) return;

    CoverSelectionDialog dialog(gridList, this);
    if (dialog.exec() == QDialog::Accepted) {
        QString selectedUrl = dialog.getSelectedUrl();
        if (!selectedUrl.isEmpty()) {
            QString coverDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/covers";
            QByteArray exePathBytes = executableName.toUtf8();
            QString uniqueId = QCryptographicHash::hash(exePathBytes, QCryptographicHash::Md5).toHex();
            QString coverPath = coverDir + "/" + uniqueId + ".png";
            m_apiManager->downloadImage(QUrl(selectedUrl), coverPath);
        }
    } else {
        m_coverChangeTargetExe.clear();
    }
}

void MainWindow::onClearHistoryClicked()
{
    if (QMessageBox::question(this, "Limpar Histórico", "Você tem certeza que deseja apagar TODOS os jogos e sessões do seu histórico? Esta ação não pode ser desfeita.", QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
        if (DatabaseManager::instance().clearAllHistory()) {
            QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/covers").removeRecursively();
            populateRecentGames();
            populateLibrary();
            QMessageBox::information(this, "Sucesso", "Seu histórico de jogos foi limpo.");
        } else {
            QMessageBox::critical(this, "Erro", "Ocorreu um erro ao tentar limpar o histórico do banco de dados.");
        }
    }
}

void MainWindow::onClearReportsClicked()
{
    if (QMessageBox::question(this, "Limpar Relatórios", "Você tem certeza que deseja apagar TODOS os arquivos de relatório salvos?", QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
        if (QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/reports").removeRecursively()) {
            QMessageBox::information(this, "Sucesso", "Todos os relatórios foram apagados.");
        } else {
            QMessageBox::critical(this, "Erro", "Não foi possível apagar a pasta de relatórios.");
        }
    }
}

void MainWindow::setActiveGameView(bool active) {
    m_activeGameWidget->setVisible(active);
    m_waitingForGameLabel->setVisible(!active);
}

void MainWindow::onNavigationButtonClicked() {
    auto* button = qobject_cast<QPushButton*>(sender());
    int index = m_navButtons.indexOf(button);
    if (index != -1) {
        m_mainStackedWidget->setCurrentIndex(index);
        updateButtonStyles(button, m_navButtons);
        updateSettingsButtonIcon(false);
    }
}

void MainWindow::onSettingsButtonClicked() {
    m_mainStackedWidget->setCurrentIndex(m_mainStackedWidget->count() - 1);
    updateButtonStyles(nullptr, m_navButtons);
    updateSettingsButtonIcon(true);
}

void MainWindow::onTempNavigationButtonClicked() {
    auto* button = qobject_cast<QPushButton*>(sender());
    int index = m_tempNavButtons.indexOf(button);
    if (index != -1) {
        m_tempStackedWidget->setCurrentIndex(index);
        updateButtonStyles(button, m_tempNavButtons);
    }
}

void MainWindow::updateButtonStyles(QPushButton *activeButton, QList<QPushButton*> &buttonGroup) {
    for (auto* btn : buttonGroup) {
        btn->setProperty("selected", (btn == activeButton));
        btn->style()->unpolish(btn);
        btn->style()->polish(btn);
    }
}

void MainWindow::updateSettingsButtonIcon(bool selected) {
    QString color = selected ? "#00d1ff" : "#94a3b8";
    QString svg = QString(R"(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="%1" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="3"></circle><path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1 0 2.83 2 2 0 0 1-2.83 0l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-2 2 2 2 0 0 1-2-2v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83 0 2 2 0 0 1 0-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1-2-2 2 2 0 0 1 2-2h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 0-2.83 2 2 0 0 1 2.83 0l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 2-2 2 2 0 0 1 2 2v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 0 2 2 0 0 1 0 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 2 2 2 2 0 0 1-2 2h-.09a1.65 1.65 0 0 0-1.51 1z"></path></svg>)").arg(color);
    QSvgRenderer renderer; renderer.load(svg.toUtf8());
    QPixmap pixmap(m_settingsButton->iconSize()); pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap); renderer.render(&painter);
    m_settingsButton->setIcon(QIcon(pixmap));
}

void MainWindow::onParticlesEnabledChanged(int state) {
    bool enabled = (static_cast<Qt::CheckState>(state) == Qt::Checked);
    QSettings settings("LAGZero", "MonitorApp");
    settings.setValue(AppConfig::SETTING_PARTICLES_ENABLED, enabled);
    m_particlesWidget->setVisible(enabled);
    if (enabled) m_particlesWidget->startAnimation(); else m_particlesWidget->stopAnimation();
}

void MainWindow::onSaveReportsChanged(int state) {
    bool enabled = (static_cast<Qt::CheckState>(state) == Qt::Checked);
    QSettings settings("LAGZero", "MonitorApp");
    settings.setValue(AppConfig::SETTING_REPORTS_ENABLED, enabled);
}

void MainWindow::onRtssStatusUpdated(bool found, const QString&) {
    m_rtssStatusCard->setVisible(!found);
}

void MainWindow::onDownloadRtssClicked() { QDesktopServices::openUrl(QUrl("https://www.guru3d.com/download/rtss-rivatuner-statistics-server-download/")); }

void MainWindow::saveSessionReport() {
    if (m_currentSession.processId == 0) return;

    QString reportsPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/reports";
    QDir dir(reportsPath);
    if (!dir.exists()) dir.mkpath(".");

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
    QString safeGameName = m_currentSession.displayName;
    safeGameName.remove(QRegularExpression(QStringLiteral("[\\/:*?\"<>|]")));

    bool isCsv = (m_reportFormatComboBox->currentIndex() == 1);
    QString extension = isCsv ? ".csv" : ".txt";
    QString filePath = reportsPath + "/" + safeGameName + "_" + timestamp + extension;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    QTextStream out(&file);

    if (isCsv) {
        out << "Métrica,Média,Máximo,Mínimo,Unidade\n";
    } else {
        out << "=============== Relatório de Sessão - LAG Zero ===============\n\n";
        out << "Jogo: " << m_currentSession.displayName << " (" << m_currentSession.exeName << ")\n";
        out << "Data: " << QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm:ss") << "\n";
        qint64 elapsed = m_currentSession.timer.elapsed() / 1000;
        QString timeString = QString("%1:%2:%3").arg(elapsed / 3600, 2, 10, QChar('0')).arg((elapsed % 3600) / 60, 2, 10, QChar('0')).arg(elapsed % 60, 2, 10, QChar('0'));
        out << "Duração da Sessão: " << timeString << "\n\n--- Resumo da Performance ---\n";
    }

    auto writeStats = [&](const QString& name, const QList<double>& data, const QString& unit) {
        if (data.isEmpty()) return;
        double sum = std::accumulate(data.begin(), data.end(), 0.0);
        double avg = sum / data.size();
        double min = *std::min_element(data.begin(), data.end());
        double max = *std::max_element(data.begin(), data.end());

        if (isCsv) {
            out << QString("%1,%2,%3,%4,%5\n").arg(name).arg(avg, 0, 'f', 1).arg(max, 0, 'f', 1).arg(min, 0, 'f', 1).arg(unit);
        } else {
            out << QString("%1:").arg(name).leftJustified(15, ' ') << QString("Méd: %1 / Máx: %2 / Mín: %3 %4\n").arg(avg, 0, 'f', 1).arg(max, 0, 'f', 1).arg(min, 0, 'f', 1).arg(unit);
        }
    };

    if (m_cpuChart) {
        writeStats("FPS", m_cpuChart->getFpsData(), "");
        writeStats("Temp. CPU", m_cpuChart->getTempData(), "C");
    }
    if (m_gpuChart) {
        writeStats("Temp. GPU", m_gpuChart->getTempData(), "C");
    }

    if (m_mbSummaryCard && m_currentSession.lastTemps.contains(AppConfig::MB_KEY)) {
        writeStats("Temp. Placa-mãe", QList<double>() << m_currentSession.lastTemps.value(AppConfig::MB_KEY).temperature, "C");
    }


    if (!isCsv) out << "\n============================================================\n";
    file.close();
}

void MainWindow::onHelperMissing()
{
    m_hardwareStatusCard->setVisible(true);
}

void MainWindow::onChartDurationChanged(int index)
{
    int duration = m_chartDurationComboBox->itemData(index).toInt();

    if (m_cpuChart) m_cpuChart->setMaxDataPoints(duration);
    if (m_gpuChart) m_gpuChart->setMaxDataPoints(duration);

    QSettings("LAGZero", "MonitorApp").setValue("chart/durationIndex", index);
}

void MainWindow::onRemoveGameRequested(const QString& executableName)
{
    GameData gameData = DatabaseManager::instance().getGameData(executableName);
    if (gameData.id == -1) return;

    if (QMessageBox::question(this, "Remover Jogo", QString("Tem certeza que deseja remover '%1' e todo o seu histórico de sessões? Esta ação não pode ser desfeita.").arg(gameData.displayName), QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
        if (DatabaseManager::instance().removeGame(executableName)) {
            if (!gameData.coverPath.isEmpty()) QFile::remove(gameData.coverPath);
            populateRecentGames();
            populateLibrary();
        } else {
            QMessageBox::critical(this, "Erro", "Não foi possível remover o jogo do banco de dados.");
        }
    }
}

void MainWindow::onManualEditRequested(const QString& executableName)
{
    GameData gameData = DatabaseManager::instance().getGameData(executableName);
    if (gameData.id == -1) return;

    bool ok;
    QString newName = QInputDialog::getText(this, "Corrigir Identificação do Jogo", "Nome correto do Jogo:", QLineEdit::Normal, gameData.displayName, &ok);

    if (ok && !newName.isEmpty()) {
        if (DatabaseManager::instance().setManualGameName(executableName, newName)) {
            populateRecentGames();
            populateLibrary();
            if (m_currentSession.exeName == executableName) {
                m_apiManager->findGameInfo(executableName, newName);
            }
        }
    }
}

void MainWindow::onHardwareUpdated(const QMap<QString, HardwareInfo> &deviceInfos)
{
    m_currentSession.lastTemps = deviceInfos;

    if (deviceInfos.contains(AppConfig::CPU_KEY)) {
        const HardwareInfo& info = deviceInfos.value(AppConfig::CPU_KEY);
        m_cpuSummaryCard->updateMetrics(info);
        if (m_currentSession.processId != 0 && info.temperature >= 0) {
            m_cpuChart->addDataPoint(info.temperature, info.usage);
        }
    }

    if (deviceInfos.contains(AppConfig::GPU_KEY)) {
        const HardwareInfo& info = deviceInfos.value(AppConfig::GPU_KEY);
        m_gpuSummaryCard->updateMetrics(info);
        if (m_currentSession.processId != 0 && info.temperature >= 0) {
            m_gpuChart->addDataPoint(info.temperature, info.usage);
        }
    }

    if (deviceInfos.contains(AppConfig::MB_KEY)) {
        const HardwareInfo& info = deviceInfos.value(AppConfig::MB_KEY);
        m_mbSummaryCard->setTitle(info.name);
        if (info.temperature >= 0) {
            m_mbSummaryCard->setValue(QString::number(info.temperature, 'f', 1) + " °C");
            m_mbSummaryCard->setValueStyleSheet("color: " + getTempColor(info.temperature, AppConfig::MB_KEY));
        } else {
            m_mbSummaryCard->setValue("Sensor não encontrado");
            m_mbSummaryCard->setValueStyleSheet("color: #aeb9d8;");
        }
    }

    QSet<QString> updatedStorageKeys;
    for (auto it = deviceInfos.constBegin(); it != deviceInfos.constEnd(); ++it) {
        if (it.key().startsWith(AppConfig::STORAGE_KEY_PREFIX)) {
            updatedStorageKeys.insert(it.key());
            const HardwareInfo& info = it.value();

            if (!m_storageCards.contains(it.key())) {
                auto* newCard = new InfoCardWidget(AppConfig::ICON_STORAGE_SVG, "Armazenamento", this);
                m_storagePageLayout->addWidget(newCard);
                m_storageCards[it.key()] = newCard;
            }

            InfoCardWidget* card = m_storageCards.value(it.key());
            if (!card) continue;

            QString title = QString("%1 (%2)").arg(info.name, info.driveType.contains("HD") ? "HD" : info.driveType);
            card->setTitle(title);

            if (info.temperature >= 0) {
                card->setValue(QString::number(info.temperature, 'f', 1) + " °C");
                card->setValueStyleSheet("color: " + getTempColor(info.temperature, "STORAGE"));
            } else {
                card->setValue("Sensor não encontrado");
                card->setValueStyleSheet("color: #aeb9d8;");
            }
        }
    }

    for (auto it = m_storageCards.begin(); it != m_storageCards.end();) {
        if (!updatedStorageKeys.contains(it.key())) {
            it.value()->deleteLater();
            it = m_storageCards.erase(it);
        } else {
            ++it;
        }
    }

    // --- INÍCIO DA REFORMULAÇÃO DA ABA DE VENTOINHAS ---
    QSet<QString> activeFanKeys;
    const int maxCols = 3;

    // 1. Limpa as posições do grid (sem deletar os widgets)
    while (m_fansLayout->count() > 0) {
        QLayoutItem* item = m_fansLayout->takeAt(0);
        if (item->widget()) {
            item->widget()->setParent(nullptr);
        }
    }
    int row = 0, col = 0;

    auto processFanMap = [&](const QMap<QString, double>& fanMap, const QString& sourceIcon, const QString& namePrefix) mutable {
        for (auto it = fanMap.constBegin(); it != fanMap.constEnd(); ++it) {
            const QString& fanName = it.key();
            double fanSpeed = it.value();

            if (fanSpeed <= 0) continue;

            QString uniqueFanName = sourceIcon + fanName;
            activeFanKeys.insert(uniqueFanName);

            // TIPO DE WIDGET CORRIGIDO
            InfoCardWidget* card = nullptr;
            if (!m_fanCards.contains(uniqueFanName)) {
                // 2. Cria um InfoCardWidget (o "quadrado" com fundo e ícone)
                QString title = namePrefix.isEmpty() ? fanName : (namePrefix + " " + fanName);
                // CORRIGIDO: Usa InfoCardWidget
                card = new InfoCardWidget(sourceIcon, title, this);
                m_fanCards[uniqueFanName] = card;
            } else {
                card = m_fanCards.value(uniqueFanName);
            }

            // 3. Readiciona o widget ao layout na posição correta
            m_fansLayout->addWidget(card, row, col++);
            card->show();
            if (col >= maxCols) {
                col = 0;
                row++;
            }

            // 4. Define o valor (RPM) e a cor
            card->setValue(QString::number(fanSpeed, 'f', 0) + " RPM");
            // CORRIGIDO: Usa setValueStyleSheet
            card->setValueStyleSheet("color: #FFFFFF;");
        }
    };

    if (deviceInfos.contains(AppConfig::CPU_KEY)) {
        processFanMap(deviceInfos.value(AppConfig::CPU_KEY).fans, AppConfig::ICON_CPU_SVG, "CPU");
    }
    if (deviceInfos.contains(AppConfig::GPU_KEY)) {
        processFanMap(deviceInfos.value(AppConfig::GPU_KEY).fans, AppConfig::ICON_GPU_SVG, "GPU");
    }
    if (deviceInfos.contains(AppConfig::MB_KEY)) {
        // 5. Ícone corrigido para ICON_FAN_SVG
        processFanMap(deviceInfos.value(AppConfig::MB_KEY).fans, AppConfig::ICON_FAN_SVG, "");
    }

    // 6. Deleta widgets que não estão mais ativos
    for (auto it = m_fanCards.begin(); it != m_fanCards.end();) {
        if (!activeFanKeys.contains(it.key())) {
            it.value()->deleteLater(); 
            it = m_fanCards.erase(it);
        } else {
            ++it;
        }
    }
    // --- FIM DA REFORMULAÇÃO DA ABA DE VENTOINHAS ---
}

void MainWindow::updateSessionInfo()
{
    if (m_currentSession.processId == 0) return;
    qint64 elapsed = m_currentSession.timer.elapsed() / 1000;
    QString timeString = QString("%1:%2:%3").arg(elapsed / 3600, 2, 10, QChar('0')).arg((elapsed % 3600) / 60, 2, 10, QChar('0')).arg(elapsed % 60, 2, 10, QChar('0'));
    m_activeGameInfoLabel->setText(QString("<b>%1</b> FPS  |  %2").arg(m_currentSession.lastFps).arg(timeString));

    auto updateMetric = [&](const QString& key, const QList<double>& data, const QString& colorKey = "") {
        if (m_sessionMetricValues.contains(key) && !data.isEmpty()) {
            double value = 0;
            if (key.startsWith("AVG")) value = std::accumulate(data.begin(), data.end(), 0.0) / data.size();
            else if (key.startsWith("MAX")) value = *std::max_element(data.begin(), data.end());

            m_sessionMetricValues[key]->setText(QString::number(value, 'f', 0));
            if (!colorKey.isEmpty()) m_sessionMetricValues[key]->setStyleSheet("color: " + getTempColor(value, colorKey));
        }
    };

    if (m_cpuChart) {
        updateMetric("AVG_FPS", m_cpuChart->getFpsData());
        updateMetric("MAX_CPU", m_cpuChart->getTempData(), AppConfig::CPU_KEY);
    }
    if (m_gpuChart) {
        updateMetric("MAX_GPU", m_gpuChart->getTempData(), AppConfig::GPU_KEY);
    }
}

void MainWindow::openReportsFolder()
{
    QString reportsPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/reports";
    QDir dir(reportsPath);
    if (!dir.exists()) dir.mkpath(".");
    QDesktopServices::openUrl(QUrl::fromLocalFile(reportsPath));
}

QWidget* MainWindow::createMetricCard(const QString& title, const QString& key) {
    auto* card = new QWidget();
    card->setProperty("class", "MetricCard");
    auto* layout = new QVBoxLayout(card);
    layout->setSpacing(2);
    auto* titleLabel = new QLabel(title); titleLabel->setProperty("class", "TitleLabel");
    auto* valueLabel = new QLabel("---"); valueLabel->setProperty("class", "ValueLabel");
    m_sessionMetricValues[key] = valueLabel;
    layout->addWidget(titleLabel);
    layout->addWidget(valueLabel);
    return card;
}

void MainWindow::onActiveGameFpsUpdate(uint32_t processId, int currentFps)
{
    if (m_currentSession.processId != processId) return;
    m_currentSession.lastFps = currentFps;
}

QString MainWindow::findEpicGameDisplayName(const QString& executablePath)
{
    QDir manifestDir("C:/ProgramData/Epic/EpicGamesLauncher/Data/Manifests");
    if (!manifestDir.exists()) return QString();

    QDirIterator it(manifestDir.absolutePath(), {"*.item"}, QDir::Files);
    while (it.hasNext()) {
        QFile file(it.next());
        if (file.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            QString displayName = doc.object()["DisplayName"].toString();
            QString installLocation = doc.object()["InstallLocation"].toString();

            if (executablePath.startsWith(installLocation, Qt::CaseInsensitive)) {
                return displayName;
            }
        }
    }
    return QString();
}
