#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QMap>
#include <QPushButton>
#include <QVBoxLayout>
#include "hardwaremonitor.h"

class FpsMonitor;

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
    // --- CORREÇÃO: Assinatura do slot atualizada para corresponder ao .cpp ---
    void onFpsUpdated(int fps, const QString& appName);

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

    // --- CORREÇÃO: Declaração dos labels de FPS que estavam faltando ---
    QLabel *m_fpsTitleLabel;
    QLabel *m_fpsValueLabel;

    QWidget* createTemperatureRow(const QString &title, QLabel* &titleLabel, QLabel* &valueLabel);
    void updateButtonStyles(QPushButton *activeButton, QList<QPushButton*> &buttonGroup);
};
#endif // MAINWINDOW_H
