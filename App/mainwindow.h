#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QMap>
#include <QPushButton>
#include <QVBoxLayout>
#include <QCheckBox>
#include "hardwaremonitor.h"
#include "particleswidget.h"

class FpsMonitor;
class QUrl;
class QFrame;

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
    void onHardwareUpdated(const QMap<QString, HardwareInfo> &deviceInfos);
    void onNavigationButtonClicked();
    void onTempNavigationButtonClicked();
    void onSettingsButtonClicked();
    void onFpsUpdated(int fps, const QString& appName);
    void onRtssStatusUpdated(bool found, const QString& installPath);
    void onDownloadRtssClicked();
    void onParticlesEnabledChanged(Qt::CheckState state);

private:
    Ui::MainWindow *ui;
    HardwareMonitor *m_hardwareMonitor;
    FpsMonitor *m_fpsMonitor;

    QStackedWidget *m_mainStackedWidget;
    QList<QPushButton*> m_navButtons;
    QPushButton *m_settingsButton;

    QStackedWidget *m_tempStackedWidget;
    QList<QPushButton*> m_tempNavButtons;

    QLabel *m_cpuTitleLabel, *m_cpuTempValueLabel;
    QLabel *m_gpuTitleLabel, *m_gpuTempValueLabel;
    QLabel *m_mbTitleLabel, *m_mbTempValueLabel;
    QMap<QString, QLabel*> m_storageTitleLabels;
    QMap<QString, QLabel*> m_storageValueLabels;
    QVBoxLayout *m_storagePageLayout;

    QLabel *m_fpsTitleLabel;
    QLabel *m_fpsValueLabel;

    // Membros para o novo cartão de alerta minimalista do RTSS
    QFrame *m_rtssStatusCard;
    QLabel *m_rtssStatusIcon;
    QLabel *m_rtssTitleLabel;
    QPushButton *m_downloadRtssButton;


    ParticlesWidget *m_particlesWidget;
    QCheckBox *m_enableParticlesCheckBox;

    QWidget* createDataRow(const QString &iconPath, const QString &title, QLabel* &titleLabel, QLabel* &valueLabel);
    void updateButtonStyles(QPushButton *activeButton, QList<QPushButton*> &buttonGroup);
    void setupSettingsPage();
    void updateSettingsButtonIcon(bool selected);
};
#endif // MAINWINDOW_H
