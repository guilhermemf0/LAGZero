#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "fpsmonitor.h"
#include "gamecoverwidget.h"
#include "launchermanager.h"
#include "databasemanager.h"
#include "appconstants.h"
#include "steamappcache.h"
#include "coverselectiondialog.h"
#include <QFontDatabase>
#include <QIcon>
#include <QStyle>
#include <QCryptographicHash>
#include <QDesktopServices>
#include <QSettings>
#include <QtSvg/QSvgRenderer>
#include <QPainter>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QTextStream>
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

QString cleanEmulatorWindowTitle(QString windowTitle) {
    if (windowTitle.contains('|')) {
        windowTitle = windowTitle.section('|', 1).trimmed();
    }

    return windowTitle;
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
    setActiveGameView(false);
    m_navButtons.first()->click();

    QSettings settings("LAGZero", "MonitorApp");
    bool particlesEnabled = settings.value(AppConfig::SETTING_PARTICLES_ENABLED, true).toBool();
    m_enableParticlesCheckBox->setChecked(particlesEnabled);
    onParticlesEnabledChanged(particlesEnabled ? Qt::Checked : Qt::Unchecked);
    m_saveReportsCheckBox->setChecked(settings.value(AppConfig::SETTING_REPORTS_ENABLED, true).toBool());
    m_chartDurationComboBox->setCurrentIndex(settings.value("chart/durationIndex", 1).toInt());
    m_reportFormatComboBox->setCurrentIndex(settings.value("reports/formatIndex", 0).toInt());
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
    m_navButtons << new QPushButton(" Visão Geral") << new QPushButton(" Temperaturas");
    for(auto btn : m_navButtons) navLayout->addWidget(btn);
    navLayout->addStretch();
    m_settingsButton = new QPushButton(); m_settingsButton->setObjectName("settingsButton");
    m_settingsButton->setCursor(Qt::PointingHandCursor);
    navLayout->addWidget(m_settingsButton, 0, Qt::AlignCenter);

    auto* contentPanel = new QFrame(); contentPanel->setObjectName("contentPanel");
    auto* contentLayout = new QVBoxLayout(contentPanel);
    m_mainStackedWidget = new QStackedWidget();
    contentLayout->addWidget(m_mainStackedWidget);
    mainLayout->addWidget(navPanel);
    mainLayout->addWidget(contentPanel, 1);

    setupOverviewPage();
    setupTempPage();
    setupSettingsPage();
}

void MainWindow::setupConnections() {
    connect(m_hardwareMonitor, &HardwareMonitor::hardwareUpdated, this, &MainWindow::onHardwareUpdated);
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
}

void MainWindow::setupOverviewPage() {
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setSpacing(20);

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

    auto* recentTitle = new QLabel("Jogos Recentes"); recentTitle->setProperty("class", "TitleLabel");
    m_recentGamesScrollArea = new QScrollArea();
    m_recentGamesScrollArea->setWidgetResizable(true);
    m_recentGamesScrollArea->setFixedHeight(240);
    m_recentGamesContainer = new QWidget();
    m_recentGamesLayout = new QHBoxLayout(m_recentGamesContainer);
    m_recentGamesLayout->setSpacing(15); m_recentGamesLayout->addStretch();
    m_recentGamesScrollArea->setWidget(m_recentGamesContainer);

    layout->addWidget(m_activeGameWidget);
    layout->addWidget(m_waitingForGameLabel);
    layout->addStretch();
    layout->addWidget(recentTitle);
    layout->addWidget(m_recentGamesScrollArea);
    m_mainStackedWidget->addWidget(page);
}

void MainWindow::setupTempPage() {
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setSpacing(15);

    auto* tempNavPanel = new QFrame(); tempNavPanel->setObjectName("tempNavPanel");
    auto* tempNavLayout = new QHBoxLayout(tempNavPanel);
    tempNavLayout->setContentsMargins(0,0,0,0);
    m_tempNavButtons << new QPushButton("CPU") << new QPushButton("GPU") << new QPushButton("Placa-mãe") << new QPushButton("Armazenamento");
    for(auto btn : m_tempNavButtons) tempNavLayout->addWidget(btn);
    tempNavLayout->addStretch();

    m_tempStackedWidget = new QStackedWidget();
    layout->addWidget(tempNavPanel);
    layout->addWidget(m_tempStackedWidget, 1);

    auto createTempPage = [&](const QString& key, const QString& icon, const QString& title) {
        auto* p = new QWidget();
        auto* l = new QVBoxLayout(p);
        l->setSpacing(20); l->setContentsMargins(0, 10, 0, 0);
        m_tempInfoCards[key] = createInfoCard(key, icon, title);
        m_charts[key] = new PerformanceChartWidget();
        l->addWidget(m_tempInfoCards[key]);
        l->addWidget(m_charts[key], 1);
        return p;
    };

    m_tempStackedWidget->addWidget(createTempPage(AppConfig::CPU_KEY, AppConfig::ICON_CPU_SVG, "Processador"));
    m_tempStackedWidget->addWidget(createTempPage(AppConfig::GPU_KEY, AppConfig::ICON_GPU_SVG, "Placa de Vídeo"));
    m_tempStackedWidget->addWidget(createTempPage(AppConfig::MB_KEY, AppConfig::ICON_MB_SVG, "Placa-mãe"));

    m_storageScrollArea = new QScrollArea(); m_storageScrollArea->setWidgetResizable(true);
    m_storageContainer = new QWidget();
    m_storagePageLayout = new QVBoxLayout(m_storageContainer);
    m_storagePageLayout->setSpacing(20); m_storagePageLayout->setAlignment(Qt::AlignTop);
    m_storageScrollArea->setWidget(m_storageContainer);
    m_tempStackedWidget->addWidget(m_storageScrollArea);

    m_mainStackedWidget->addWidget(page);
}

void MainWindow::setupSettingsPage() {
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setSpacing(15); layout->setAlignment(Qt::AlignTop);
    auto* title = new QLabel("Configurações"); title->setProperty("class", "TitleLabel");
    layout->addWidget(title);

    auto* appearanceTitle = new QLabel("Aparência"); appearanceTitle->setProperty("class", "SubtitleLabel");
    layout->addWidget(appearanceTitle);
    m_enableParticlesCheckBox = new QCheckBox("Ativar efeito de partículas no fundo");
    layout->addWidget(m_enableParticlesCheckBox);

    m_chartDurationComboBox = new QComboBox();
    m_chartDurationComboBox->addItem("Últimos 60 segundos", 60);
    m_chartDurationComboBox->addItem("Últimos 2 minutos", 120);
    m_chartDurationComboBox->addItem("Últimos 5 minutos", 300);
    auto* chartLayout = new QHBoxLayout();
    chartLayout->addWidget(new QLabel("Período de tempo do gráfico:"));
    chartLayout->addWidget(m_chartDurationComboBox);
    layout->addLayout(chartLayout);

    auto* reportsTitle = new QLabel("Relatórios"); reportsTitle->setProperty("class", "SubtitleLabel");
    layout->addWidget(reportsTitle);
    m_saveReportsCheckBox = new QCheckBox("Salvar relatórios de performance da sessão");
    layout->addWidget(m_saveReportsCheckBox);

    m_reportFormatComboBox = new QComboBox();
    m_reportFormatComboBox->addItem("Texto (.txt)");
    m_reportFormatComboBox->addItem("CSV (.csv)");
    auto* reportFormatLayout = new QHBoxLayout();
    reportFormatLayout->addWidget(new QLabel("Formato do relatório:"));
    reportFormatLayout->addWidget(m_reportFormatComboBox);
    layout->addLayout(reportFormatLayout);

    auto* reportsBtn = new QPushButton("Abrir pasta de relatórios");
    reportsBtn->setCursor(Qt::PointingHandCursor);
    reportsBtn->setStyleSheet("background:transparent; color:#0085ff; text-decoration:underline; font-weight: 600;");
    connect(reportsBtn, &QPushButton::clicked, this, &MainWindow::openReportsFolder);
    layout->addWidget(reportsBtn, 0, Qt::AlignLeft);

    layout->addStretch();
    m_mainStackedWidget->addWidget(page);
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
        qDebug() << "[SESSION START] Associação manual encontrada no DB:" << searchName;
    } else if (emulatorExes.contains(currentExeFile)) {
        searchName = cleanEmulatorWindowTitle(windowTitle);
        qDebug() << "[SESSION START] Emulador detectado. Usando título da janela como nome:" << searchName;
    } else {
        QString accurateGameName = LauncherManager::instance().findGameDisplayName(exeName, processId);
        if (!accurateGameName.isEmpty()) {
            searchName = accurateGameName;
        } else {
            searchName = windowTitle.isEmpty() ? currentExeFile : windowTitle;
        }
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
        qDebug() << "[SESSION START] Jogo completo encontrado no DB:" << existingData.displayName;
    } else {
        m_currentSession.displayName = searchName;
        m_activeGameNameLabel->setText(searchName);
        m_activeGameCoverLabel->clear();
        qDebug() << "[SESSION START] Jogo novo ou incompleto. Buscando online com o nome:" << searchName;
        m_apiManager->findGameInfo(exeName, searchName);
    }

    for(auto* chart : m_charts) if(chart) chart->clearData();
    m_sessionTimer->start(1000);
    m_currentSession.timer.start();
}

void MainWindow::onApiSearchFinished(const ApiGameResult& result)
{
    qDebug() << "[API FINISHED] Para o executável:" << result.executableName;
    if (result.executableName != m_currentSession.exeName) {
        qDebug() << "[API FINISHED] Resultado é de uma sessão antiga. Ignorando.";
        return;
    }

    QString finalDisplayName = result.name.isEmpty() ? m_currentSession.displayName : result.name;
    qDebug() << "[API FINISHED] Nome final determinado:" << finalDisplayName;

    m_activeGameNameLabel->setText(finalDisplayName);
    m_currentSession.displayName = finalDisplayName;

    if (result.success) {
        qDebug() << "[API FINISHED] Busca bem-sucedida. URL da capa:" << result.coverUrl;
        QString coverDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/covers";
        if (!QDir(coverDir).exists()) QDir().mkpath(coverDir);

        // --- MUDANÇA FINAL PARA NOMES DE ARQUIVO ÚNICOS ---
        // Criamos um hash MD5 do caminho completo do executável.
        // Isso gera um nome de arquivo curto e 100% único para cada jogo,
        // resolvendo todas as colisões de uma vez por todas.
        QByteArray exePathBytes = result.executableName.toUtf8();
        QString uniqueId = QCryptographicHash::hash(exePathBytes, QCryptographicHash::Md5).toHex();

        QString coverFileName = uniqueId + ".png";
        QString coverPath = coverDir + "/" + coverFileName;

        qDebug() << "[DEBUG] Executável:" << result.executableName << "-> ID Único:" << uniqueId;

        DatabaseManager::instance().addOrUpdateGame(result.executableName, finalDisplayName, "");
        m_apiManager->downloadImage(QUrl(result.coverUrl), coverPath);
    } else {
        qDebug() << "[API FINISHED] Busca falhou. Salvando jogo com o melhor nome que temos, mas sem capa.";
        DatabaseManager::instance().addOrUpdateGame(result.executableName, finalDisplayName, "");
    }
}

void MainWindow::onImageDownloaded(const QString& localPath, const QUrl&)
{
    qDebug() << "[IMAGE DOWNLOADED] Caminho:" << localPath;

    QString coverFileId = QFileInfo(localPath).baseName();

    QByteArray currentSessionExePathBytes = m_currentSession.exeName.toUtf8();
    QString currentSessionId = QCryptographicHash::hash(currentSessionExePathBytes, QCryptographicHash::Md5).toHex();

    if (coverFileId != currentSessionId) {
        qDebug() << "[IMAGE DOWNLOADED] Imagem é de uma sessão antiga. Ignorando. (ID da Capa:" << coverFileId << "| ID da Sessão:" << currentSessionId << ")";
        return;
    }

    m_currentSession.coverPath = localPath;
    m_activeGameCoverLabel->setPixmap(QPixmap(localPath));

    int gameId = DatabaseManager::instance().getGameId(m_currentSession.exeName);
    if (gameId != -1) {
        qDebug() << "[IMAGE DOWNLOADED] Atualizando caminho da capa no DB.";
        DatabaseManager::instance().updateGameCover(gameId, localPath);
    }
}

void MainWindow::onEditGameRequested(const QString& executableName)
{
    GameData gameData = DatabaseManager::instance().getGameData(executableName);
    if (gameData.id == -1) return;

    bool ok;
    QString newName = QInputDialog::getText(this, "Editar Jogo", "Nome de Exibição:", QLineEdit::Normal, gameData.displayName, &ok);

    if (ok && !newName.isEmpty()) {
        QString newCoverPath = QFileDialog::getOpenFileName(this, "Selecionar Nova Capa", "", "Imagens (*.png *.jpg *.jpeg)");

        if (!newCoverPath.isEmpty()) {
            DatabaseManager::instance().addOrUpdateGame(gameData.executableName, newName, newCoverPath);
        } else {
            DatabaseManager::instance().addOrUpdateGame(gameData.executableName, newName, gameData.coverPath);
        }
        populateRecentGames();
    }
}

void MainWindow::onHardwareUpdated(const QMap<QString, HardwareInfo> &deviceInfos)
{
    for (auto it = deviceInfos.constBegin(); it != deviceInfos.constEnd(); ++it) {
        m_currentSession.lastTemps[it.key()] = it.value().temperature;

        if (m_tempInfoCards.contains(it.key())) {
            auto* card = m_tempInfoCards.value(it.key());
            auto* valueLabel = card->findChild<QLabel*>("Value");
            auto* titleLabel = card->findChild<QLabel*>("Title");
            if (valueLabel && it.value().temperature >= 0) {
                valueLabel->setText(QString::number(it.value().temperature, 'f', 1) + " °C");
                valueLabel->setStyleSheet("color: " + getTempColor(it.value().temperature, it.key()));
            }
            if (titleLabel && !it.value().name.isEmpty() && it.value().name != "N/D") {
                titleLabel->setText(it.value().name);
            }
        } else if (it.key().startsWith(AppConfig::STORAGE_KEY_PREFIX)) {
            if (!m_tempInfoCards.contains(it.key())) {
                m_storagePageLayout->addWidget(createInfoCard(it.key(), AppConfig::ICON_STORAGE_SVG, it.value().name));
            }
        }
    }

    if (m_currentSession.processId != 0) {
        for (auto it = m_charts.constBegin(); it != m_charts.constEnd(); ++it) {
            if (m_currentSession.lastTemps.contains(it.key())) {
                it.value()->addDataPoint(m_currentSession.lastTemps.value(it.key()), m_currentSession.lastFps);
            }
        }
    }
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
    }
    m_currentSession = CurrentSession();
}

void MainWindow::onActiveGameFpsUpdate(uint32_t processId, int currentFps)
{
    if (m_currentSession.processId != processId) return;
    m_currentSession.lastFps = currentFps;
}

void MainWindow::updateSessionInfo()
{
    if (m_currentSession.processId == 0) return;
    qint64 elapsed = m_currentSession.timer.elapsed() / 1000;
    QString timeString = QString("%1:%2:%3")
                             .arg(elapsed / 3600, 2, 10, QChar('0'))
                             .arg((elapsed % 3600) / 60, 2, 10, QChar('0'))
                             .arg(elapsed % 60, 2, 10, QChar('0'));

    m_activeGameInfoLabel->setText(QString("<b>%1</b> FPS  |  %2").arg(m_currentSession.lastFps).arg(timeString));

    auto updateMetric = [&](const QString& key, const QList<double>& data, const QString& colorKey = "") {
        if (m_sessionMetricValues.contains(key) && !data.isEmpty()) {
            double value = 0;
            if (key.startsWith("AVG")) value = std::accumulate(data.begin(), data.end(), 0.0) / data.size();
            else if (key.startsWith("MAX")) value = *std::max_element(data.begin(), data.end());

            m_sessionMetricValues[key]->setText(QString::number(value, 'f', 0));
            if (!colorKey.isEmpty()) {
                m_sessionMetricValues[key]->setStyleSheet("color: " + getTempColor(value, colorKey));
            }
        }
    };

    if (m_charts.contains(AppConfig::CPU_KEY)) {
        updateMetric("AVG_FPS", m_charts.value(AppConfig::CPU_KEY)->getFpsData());
        updateMetric("MAX_CPU", m_charts.value(AppConfig::CPU_KEY)->getTempData(), AppConfig::CPU_KEY);
    }
    if (m_charts.contains(AppConfig::GPU_KEY)) {
        updateMetric("MAX_GPU", m_charts.value(AppConfig::GPU_KEY)->getTempData(), AppConfig::GPU_KEY);
    }
}

void MainWindow::openReportsFolder()
{
    QString reportsPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/reports";
    QDir dir(reportsPath);
    if (!dir.exists()) dir.mkpath(".");
    QDesktopServices::openUrl(QUrl::fromLocalFile(reportsPath));
}

QWidget* MainWindow::createInfoCard(const QString& key, const QString& iconSvg, const QString& title) {
    auto* card = new QWidget();
    card->setProperty("class", "InfoCard");
    card->setFixedHeight(100);
    auto* layout = new QHBoxLayout(card);
    auto* icon = new QLabel();
    icon->setFixedSize(32, 32);
    QSvgRenderer renderer; renderer.load(iconSvg.toUtf8());
    QPixmap pixmap(icon->size()); pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap); renderer.render(&painter);
    icon->setPixmap(pixmap);

    auto* textLayout = new QVBoxLayout();
    auto* titleLabel = new QLabel(title); titleLabel->setProperty("class", "Title");
    titleLabel->setObjectName("Title");
    auto* valueLabel = new QLabel("N/D"); valueLabel->setProperty("class", "Value");
    valueLabel->setObjectName("Value");
    textLayout->addWidget(titleLabel);
    textLayout->addWidget(valueLabel);

    layout->addWidget(icon);
    layout->addLayout(textLayout);
    m_tempInfoCards[key] = card;
    return card;
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

void MainWindow::populateRecentGames() {
    QLayoutItem* item;
    while ((item = m_recentGamesLayout->takeAt(0)) != nullptr) {
        if (item->widget()) delete item->widget();
        delete item;
    }
    QList<GameData> recentGames = DatabaseManager::instance().getGamesByMostRecent();
    for (const auto& gameData : recentGames) {
        QPixmap cover(gameData.coverPath);
        auto* coverWidget = new GameCoverWidget(gameData.displayName, gameData.executableName, cover);
        connect(coverWidget, &GameCoverWidget::editGameRequested, this, &MainWindow::onManualEditRequested);
        connect(coverWidget, &GameCoverWidget::removeGameRequested, this, &MainWindow::onRemoveGameRequested);
        connect(coverWidget, &GameCoverWidget::changeCoverRequested, this, &MainWindow::triggerCoverChange);
        m_recentGamesLayout->insertWidget(m_recentGamesLayout->count() - 1, coverWidget);
    }
    m_recentGamesLayout->addStretch();
}

void MainWindow::triggerCoverChange(const QString& executableName)
{
    GameData gameData = DatabaseManager::instance().getGameData(executableName);
    if (gameData.id == -1) return;

    m_apiManager->findGameInfo(executableName, gameData.displayName);
}

void MainWindow::onGridListReady(const QString& executableName, const QList<QJsonObject>& gridList)
{
    if (m_currentSession.exeName != executableName && !m_currentSession.exeName.isEmpty()) {
        return;
    }

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
    QString svg = QString(R"(<svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="%1" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="3"></circle><path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1 0 2.83 2 2 0 0 1-2.83 0l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-2 2 2 2 0 0 1-2-2v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83 0 2 2 0 0 1 0-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1-2-2 2 2 0 0 1 2-2h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 0-2.83 2 2 0 0 1 2.83 0l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 2-2 2 2 0 0 1 2 2v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 0 2 2 0 0 1 0 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 2 2 2 2 0 0 1-2 2h-.09a1.65 1.65 0 0 0-1.51 1z"></path></svg>)").arg(color);
    QSvgRenderer renderer; renderer.load(svg.toUtf8());
    QPixmap pixmap(m_settingsButton->size()); pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap); renderer.render(&painter);
    m_settingsButton->setIcon(QIcon(pixmap));
    m_settingsButton->setIconSize(QSize(20,20));
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
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Não foi possível criar o arquivo de relatório:" << filePath;
        return;
    }

    QTextStream out(&file);

    if (isCsv) {
        out << "Métrica,Média,Máximo,Mínimo,Unidade\n";
    } else {
        out << "=============== Relatório de Sessão - LAG Zero ===============\n\n";
        out << "Jogo: " << m_currentSession.displayName << " (" << m_currentSession.exeName << ")\n";
        out << "Data: " << QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm:ss") << "\n";
        qint64 elapsed = m_currentSession.timer.elapsed() / 1000;
        QString timeString = QString("%1:%2:%3")
                                 .arg(elapsed / 3600, 2, 10, QChar('0'))
                                 .arg((elapsed % 3600) / 60, 2, 10, QChar('0'))
                                 .arg(elapsed % 60, 2, 10, QChar('0'));
        out << "Duração da Sessão: " << timeString << "\n";
        out << "\n--- Resumo da Performance ---\n";
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
            QString line = QString("%1:").arg(name).leftJustified(15, ' ');
            line += QString("Méd: %1 / Máx: %2 / Mín: %3 %4\n")
                        .arg(avg, 0, 'f', 1)
                        .arg(max, 0, 'f', 1)
                        .arg(min, 0, 'f', 1)
                        .arg(unit);
            out << line;
        }
    };

    if (m_charts.contains(AppConfig::CPU_KEY)) {
        writeStats("FPS", m_charts.value(AppConfig::CPU_KEY)->getFpsData(), "");
        for (auto it = m_charts.constBegin(); it != m_charts.constEnd(); ++it) {
            QString key = it.key();
            PerformanceChartWidget* chart = it.value();
            QString formattedKey;
            if (key == AppConfig::CPU_KEY) formattedKey = "Temp. CPU";
            else if (key == AppConfig::GPU_KEY) formattedKey = "Temp. GPU";
            else if (key == AppConfig::MB_KEY) formattedKey = "Temp. Placa-mãe";
            else if (key.startsWith(AppConfig::STORAGE_KEY_PREFIX)) formattedKey = "Temp. Drive";

            if(!formattedKey.isEmpty()){
                writeStats(formattedKey, chart->getTempData(), "C");
            }
        }
    }

    if (!isCsv) {
        out << "\n============================================================\n";
    }
    file.close();
}

void MainWindow::onHelperMissing()
{
    m_hardwareStatusCard->setVisible(true);
}

void MainWindow::onChartDurationChanged(int index)
{
    int duration = m_chartDurationComboBox->itemData(index).toInt();
    for (auto* chart : m_charts) {
        if (chart) {
            chart->setMaxDataPoints(duration);
        }
    }
    QSettings settings("LAGZero", "MonitorApp");
    settings.setValue("chart/durationIndex", index);
}

void MainWindow::onRemoveGameRequested(const QString& executableName)
{
    GameData gameData = DatabaseManager::instance().getGameData(executableName);
    if (gameData.id == -1) return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Remover Jogo",
                                  QString("Tem certeza que deseja remover '%1' e todo o seu histórico de sessões? Esta ação não pode ser desfeita.").arg(gameData.displayName),
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        if (DatabaseManager::instance().removeGame(executableName)) {
            if (!gameData.coverPath.isEmpty()) {
                QFile::remove(gameData.coverPath);
            }
            populateRecentGames();
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
    QString newName = QInputDialog::getText(this, "Corrigir Identificação do Jogo",
                                            "Nome correto do Jogo:", QLineEdit::Normal,
                                            gameData.displayName, &ok);

    if (ok && !newName.isEmpty()) {
        if (DatabaseManager::instance().setManualGameName(executableName, newName)) {
            // Sucesso! Limpamos a lista de jogos recentes e a populamos novamente
            // para forçar a atualização da UI e uma nova busca de capa.
            populateRecentGames();

            // Se o jogo que editamos é o que está ativo, reinicia a busca
            if (m_currentSession.exeName == executableName) {
                m_apiManager->findGameInfo(executableName, newName);
            }
        }
    }
}
