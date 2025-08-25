#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QMap>
#include <QPushButton>
#include <QVBoxLayout>
#include "hardwaremonitor.h"

class FpsMonitor; // Forward declaration
class QUrl; // Forward declaration para QUrl

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
    void onFpsUpdated(int fps, const QString& appName);
    // NOVO: Slot para receber o status do RTSS
    void onRtssStatusUpdated(bool found, const QString& installPath);
    // NOVO: Slot para lidar com o clique no botão de download do RTSS
    void onDownloadRtssClicked();

private:
    Ui::MainWindow *ui;
    HardwareMonitor *m_hardwareMonitor;
    FpsMonitor *m_fpsMonitor;

    QStackedWidget *m_mainStackedWidget;
    QList<QPushButton*> m_navButtons;

    QStackedWidget *m_tempStackedWidget;
    QList<QPushButton*> m_tempNavButtons;

    // Labels para as temperaturas
    QLabel *m_cpuTitleLabel, *m_cpuTempValueLabel;
    QLabel *m_gpuTitleLabel, *m_gpuTempValueLabel;
    QLabel *m_mbTitleLabel, *m_mbTempValueLabel;
    QMap<QString, QLabel*> m_storageTitleLabels;
    QMap<QString, QLabel*> m_storageValueLabels;
    QVBoxLayout *m_storagePageLayout;

    // Labels para o FPS
    QLabel *m_fpsTitleLabel;
    QLabel *m_fpsValueLabel;

    // NOVO: Widgets para a mensagem de status do RTSS
    QLabel *m_rtssStatusLabel;
    QPushButton *m_downloadRtssButton;
    QWidget *m_rtssStatusWidget; // Widget para agrupar a mensagem e o botão

    QWidget* createTemperatureRow(const QString &title, QLabel* &titleLabel, QLabel* &valueLabel);
    void updateButtonStyles(QPushButton *activeButton, QList<QPushButton*> &buttonGroup);
};
#endif // MAINWINDOW_H
