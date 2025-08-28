#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <cstdint>
#include <QMap>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QScrollArea>
#include <QFrame>
#include <QUrl>
#include <QList>
#include <QTimer>
#include <QElapsedTimer>

#include "hardwaremonitor.h"
#include "particleswidget.h"
#include "databasemanager.h"
#include "apimanager.h"
#include "performancechartwidget.h"
#include "appconstants.h"

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
    void onGameSessionStarted(const QString& exeName, uint32_t processId);
    void onGameSessionEnded(uint32_t processId, const QString& exeName, double averageFps);
    void onActiveGameFpsUpdate(uint32_t processId, int currentFps);
    void onApiSearchFinished(const ApiGameResult& result);
    void onImageDownloaded(const QString& localPath, const QUrl& originalUrl);
    void updateSessionDisplay();
    void openReportsFolder(); // Novo slot

private:
    Ui::MainWindow *ui;
    HardwareMonitor *m_hardwareMonitor;
    FpsMonitor *m_fpsMonitor;
    ApiManager *m_apiManager;

    QStackedWidget *m_mainStackedWidget;
    QList<QPushButton*> m_navButtons;
    QPushButton *m_settingsButton;
    ParticlesWidget *m_particlesWidget;

    QWidget* m_activeGameWidget;
    QLabel* m_activeGameCoverLabel;
    QLabel* m_activeGameNameLabel;
    QLabel* m_activeGameFpsLabel;
    QLabel* m_waitingForGameLabel;
    QScrollArea* m_recentGamesScrollArea;
    QWidget* m_recentGamesContainer;
    QHBoxLayout* m_recentGamesLayout;

    QStackedWidget *m_tempStackedWidget;
    QList<QPushButton*> m_tempNavButtons;

    QMap<QString, QLabel*> m_tempTitleLabels;
    QMap<QString, QLabel*> m_tempValueLabels;
    QMap<QString, QLabel*> m_tempFpsValueLabels;

    // Widgets para a aba de armazenamento
    QScrollArea* m_storageScrollArea;
    QWidget* m_storageContainer;
    QVBoxLayout *m_storagePageLayout;
    QMap<QString, QLabel*> m_storageTitleLabels;
    QMap<QString, QLabel*> m_storageValueLabels;
    QMap<QString, PerformanceChartWidget*> m_storageCharts; // Novo

    QMap<QString, PerformanceChartWidget*> m_charts;
    QMap<QString, QWidget*> m_gameInfoWidgets;
    QMap<QString, QLabel*> m_sessionTimeLabels;

    QCheckBox *m_enableParticlesCheckBox;
    QCheckBox *m_saveReportsCheckBox;

    QFrame *m_rtssStatusCard;
    QLabel *m_rtssStatusIcon;
    QLabel *m_rtssTitleLabel;
    QPushButton *m_downloadRtssButton;

    CurrentSession m_currentSession;
    QTimer* m_sessionTimer;

    void setupUi();
    void setupConnections();
    void setupOverviewPage();
    void setupTempPage();
    void setupSettingsPage();
    void populateRecentGames();
    void setActiveGameView(bool active);
    QWidget* createDataRow(const QString &iconPath, const QString &title, QLabel* &titleLabel, QLabel* &valueLabel);
    void updateButtonStyles(QPushButton *activeButton, QList<QPushButton*> &buttonGroup);
    void updateSettingsButtonIcon(bool selected);
    void updateTempGameInfo(bool isGameRunning);
    void saveSessionReport();
};
#endif // MAINWINDOW_H
