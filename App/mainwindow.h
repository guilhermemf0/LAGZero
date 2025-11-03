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
#include "apimanager.h"
#include "performancechartwidget.h"
#include "gamecoverwidget.h"
#include "infocardwidget.h"
#include "hardwaresummarycard.h"
#include "overlaypositionselector.h"
#include "overlaystyleselector.h"

class FpsMonitor;
class QButtonGroup;

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
    QMap<QString, HardwareInfo> lastTemps;
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
    void onGameSessionStarted(const QString& exeName, const QString& windowTitle, uint32_t processId);
    void onGameSessionEnded(uint32_t processId, const QString& exeName, double averageFps);
    void onActiveGameFpsUpdate(uint32_t processId, int currentFps);
    void onApiSearchFinished(const ApiGameResult& result);
    void onImageDownloaded(const QString& localPath, const QUrl& originalUrl);
    void openReportsFolder();
    void updateSessionInfo();
    void onHelperMissing();
    void onChartDurationChanged(int index);
    void onRemoveGameRequested(const QString& executableName);
    void onManualEditRequested(const QString& executableName);
    void onGridListReady(const QString& executableName, const QList<QJsonObject>& gridList);
    void onClearHistoryClicked();
    void onClearReportsClicked();
    void onOverlaySettingChanged();
    void onOverlayPositionChanged(int id);
    void onOverlayStyleChanged(int id);


private:
    Ui::MainWindow *ui;
    HardwareMonitor *m_hardwareMonitor;
    FpsMonitor *m_fpsMonitor;
    ApiManager *m_apiManager;
    ParticlesWidget *m_particlesWidget;
    QString m_coverChangeTargetExe;

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

    QScrollArea* m_libraryScrollArea;
    QWidget* m_libraryContainer;
    QGridLayout* m_libraryLayout;

    QStackedWidget *m_tempStackedWidget;
    QList<QPushButton*> m_tempNavButtons;
    HardwareSummaryCard* m_cpuSummaryCard;
    HardwareSummaryCard* m_gpuSummaryCard;
    InfoCardWidget* m_mbSummaryCard;
    PerformanceChartWidget* m_cpuChart;
    PerformanceChartWidget* m_gpuChart;
    QScrollArea* m_storageScrollArea;
    QWidget* m_storageContainer;
    QVBoxLayout *m_storagePageLayout;
    QMap<QString, InfoCardWidget*> m_storageCards;

    QScrollArea* m_fansScrollArea;
    QWidget* m_fansContainer;
    QGridLayout* m_fansLayout;
    // TIPO DE WIDGET CORRIGIDO (revertido para InfoCardWidget)
    QMap<QString, InfoCardWidget*> m_fanCards;

    QCheckBox *m_enableParticlesCheckBox;
    QCheckBox *m_saveReportsCheckBox;
    QComboBox *m_reportFormatComboBox;
    QComboBox *m_chartDurationComboBox;

    QCheckBox* m_overlayEnabledCheckBox;
    OverlayPositionSelector* m_positionSelector;
    OverlayStyleSelector* m_styleSelector;

    QCheckBox* m_overlayShowCpuTempCheckBox;
    QCheckBox* m_overlayShowCpuUsageCheckBox;
    QCheckBox* m_overlayShowCpuCoresCheckBox;
    QCheckBox* m_overlayShowGpuTempCheckBox;
    QCheckBox* m_overlayShowGpuUsageCheckBox;
    QCheckBox* m_overlayShowRamUsageCheckBox;
    QCheckBox* m_overlayShowMbTempCheckBox;
    QCheckBox* m_overlayShowStorageTempCheckBox;
    QCheckBox* m_overlayShowAvgFpsCheckBox;
    QCheckBox* m_overlayShowMinFpsCheckBox;
    QCheckBox* m_overlayShowMaxFpsCheckBox;

    QCheckBox* m_overlayShowCpuPowerCheckBox;
    QCheckBox* m_overlayShowCpuClockCheckBox;
    QCheckBox* m_overlayShowGpuPowerCheckBox;
    QCheckBox* m_overlayShowGpuClockCheckBox;
    QCheckBox* m_overlayShowFansCheckBox;


    QFrame *m_rtssStatusCard;
    QFrame *m_hardwareStatusCard;

    CurrentSession m_currentSession;
    QTimer* m_sessionTimer;

    void setupUi();
    void setupConnections();
    void setupOverviewPage();
    void setupLibraryPage();
    void setupTempPage();
    void setupFansPage();
    void setupSettingsPage();
    QWidget* createMetricCard(const QString& title, const QString& key);

    void populateRecentGames();
    void populateLibrary();
    void setActiveGameView(bool active);
    void updateButtonStyles(QPushButton *activeButton, QList<QPushButton*> &buttonGroup);
    void updateSettingsButtonIcon(bool selected);
    void saveSessionReport();
    QString findEpicGameDisplayName(const QString& executablePath);
    void triggerCoverChange(const QString& executableName);
};
#endif // MAINWINDOW_H
