#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <cstdint>
#include <QMap>

// CORREÇÃO: Inclusão completa de todos os cabeçalhos de widgets e layouts necessários
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

#include "hardwaremonitor.h"
#include "particleswidget.h"
#include "databasemanager.h"
#include "apimanager.h"

// Forward declarations para classes que não precisam de definição completa no header
class FpsMonitor;
class GameCoverWidget;
class QListWidgetItem;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Slots de monitores e sistema
    void onHardwareUpdated(const QMap<QString, HardwareInfo> &deviceInfos);
    void onRtssStatusUpdated(bool found, const QString& installPath);
    void onDownloadRtssClicked();

    // Slots de navegação e UI
    void onNavigationButtonClicked();
    void onTempNavigationButtonClicked();
    void onSettingsButtonClicked();
    void onParticlesEnabledChanged(int state);

    // Slots para gerenciamento de jogos
    void onGameSessionStarted(const QString& exeName, uint32_t processId);
    void onGameSessionEnded(uint32_t processId, const QString& exeName, double averageFps);
    void onActiveGameFpsUpdate(uint32_t processId, int currentFps);
    void onApiSearchFinished(const ApiGameResult& result);
    void onImageDownloaded(const QString& localPath, const QUrl& originalUrl);

private:
    Ui::MainWindow *ui;
    HardwareMonitor *m_hardwareMonitor;
    FpsMonitor *m_fpsMonitor;
    ApiManager *m_apiManager;

    // --- Estrutura da UI Principal ---
    QStackedWidget *m_mainStackedWidget;
    QList<QPushButton*> m_navButtons;
    QPushButton *m_settingsButton;
    ParticlesWidget *m_particlesWidget;

    // --- Widgets da Página "Visão Geral" (Nova UI) ---
    QWidget* m_activeGameWidget;
    QLabel* m_activeGameCoverLabel;
    QLabel* m_activeGameNameLabel;
    QLabel* m_activeGameFpsLabel;
    QLabel* m_waitingForGameLabel;
    QScrollArea* m_recentGamesScrollArea;
    QWidget* m_recentGamesContainer;
    QHBoxLayout* m_recentGamesLayout;

    // --- Widgets da Página "Temperaturas" ---
    QStackedWidget *m_tempStackedWidget;
    QList<QPushButton*> m_tempNavButtons;
    QLabel *m_cpuTitleLabel, *m_cpuTempValueLabel;
    QLabel *m_gpuTitleLabel, *m_gpuTempValueLabel;
    QLabel *m_mbTitleLabel, *m_mbTempValueLabel;
    QMap<QString, QLabel*> m_storageTitleLabels;
    QMap<QString, QLabel*> m_storageValueLabels;
    QVBoxLayout *m_storagePageLayout;

    // --- Widgets da Página "Configurações" ---
    QCheckBox *m_enableParticlesCheckBox;

    // --- Widgets de Alerta ---
    QFrame *m_rtssStatusCard;
    QLabel *m_rtssStatusIcon;
    QLabel *m_rtssTitleLabel;
    QPushButton *m_downloadRtssButton;

    // --- Funções Auxiliares ---
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
};
#endif // MAINWINDOW_H
