#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <cstdint>
#include <QMap>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QScrollArea>
#include <QFrame>
#include <QUrl>
#include <QList>
#include <QTimer>
#include <QElapsedTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QComboBox>

#include "hardwaremonitor.h"
#include "particleswidget.h"
#include "databasemanager.h"
#include "apimanager.h"
#include "performancechartwidget.h"
#include "appconstants.h"
#include "steamappcache.h"

class FpsMonitor;
class GameCoverWidget;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct CurrentSession {
    uint32_t processId = 0;
    QString exeName;
    QString displayName;
    QString coverPath;
    QElapsedTimer timer;
    int lastFps = 0;
    QMap<QString, double> lastTemps;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onHardwareUpdated(const QMap<QString, HardwareInfo> &deviceInfos);
    void onRtssStatusUpdated(bool found, const QString& installPath);
    void onDownloadRtssClicked();
    void onNavigationButtonClicked();
    void onTempNavigationButtonClicked();
    void onSettingsButtonClicked();
    void onParticlesEnabledChanged(int state);
    void onSaveReportsChanged(int state);
    // ALTERADO: Assinatura do slot para receber o título da janela
    void onGameSessionStarted(const QString& exeName, const QString& windowTitle, uint32_t processId);
    void onGameSessionEnded(uint32_t processId, const QString& exeName, double averageFps);
    void onActiveGameFpsUpdate(uint32_t processId, int currentFps);
    void onApiSearchFinished(const ApiGameResult& result);
    void onImageDownloaded(const QString& localPath, const QUrl& originalUrl);
    void openReportsFolder();
    void updateSessionInfo();
    void onHelperMissing();
    void onChartDurationChanged(int index);
    void onEditGameRequested(const QString& executableName);
    void onRemoveGameRequested(const QString& executableName);

private:
    Ui::MainWindow *ui;
    HardwareMonitor *m_hardwareMonitor;
    FpsMonitor *m_fpsMonitor;
    ApiManager *m_apiManager;
    ParticlesWidget *m_particlesWidget;

    QStackedWidget *m_mainStackedWidget;
    QList<QPushButton*> m_navButtons;
    QPushButton *m_settingsButton;

    QWidget* m_activeGameWidget;
    QLabel* m_activeGameCoverLabel;
    QLabel* m_activeGameNameLabel;
    QLabel* m_activeGameInfoLabel;
    QLabel* m_waitingForGameLabel;
    QScrollArea* m_recentGamesScrollArea;
    QWidget* m_recentGamesContainer;
    QHBoxLayout* m_recentGamesLayout;
    QMap<QString, QLabel*> m_sessionMetricValues;

    QStackedWidget *m_tempStackedWidget;
    QList<QPushButton*> m_tempNavButtons;
    QMap<QString, PerformanceChartWidget*> m_charts;
    QMap<QString, QWidget*> m_tempInfoCards;
    QScrollArea* m_storageScrollArea;
    QWidget* m_storageContainer;
    QVBoxLayout *m_storagePageLayout;

    QCheckBox *m_enableParticlesCheckBox;
    QCheckBox *m_saveReportsCheckBox;
    QComboBox *m_reportFormatComboBox;
    QComboBox *m_chartDurationComboBox;

    QFrame *m_rtssStatusCard;
    QFrame *m_hardwareStatusCard;

    CurrentSession m_currentSession;
    QTimer* m_sessionTimer;

    void setupUi();
    void setupConnections();
    void setupOverviewPage();
    void setupTempPage();
    void setupSettingsPage();
    QWidget* createInfoCard(const QString& key, const QString& iconSvg, const QString& title);
    QWidget* createMetricCard(const QString& title, const QString& key);

    void populateRecentGames();
    void setActiveGameView(bool active);
    void updateButtonStyles(QPushButton *activeButton, QList<QPushButton*> &buttonGroup);
    void updateSettingsButtonIcon(bool selected);
    void saveSessionReport();
};
#endif // MAINWINDOW_H
