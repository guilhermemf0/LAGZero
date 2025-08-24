#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QMap>
#include <QPushButton>
#include <QVBoxLayout>
#include "cputemperature.h" // Incluído para a struct HardwareInfo

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
    // --- CORREÇÃO: Assinatura do slot atualizada ---
    void onTemperaturesUpdated(const QMap<QString, HardwareInfo> &deviceInfos);
    void onNavigationButtonClicked();
    void onTempNavigationButtonClicked();

private:
    Ui::MainWindow *ui;
    CpuTemperature *m_tempReader;

    QStackedWidget *m_mainStackedWidget;
    QList<QPushButton*> m_navButtons;

    QStackedWidget *m_tempStackedWidget;
    QList<QPushButton*> m_tempNavButtons;

    // --- CORREÇÃO: Declaração dos ponteiros de labels que estavam faltando ---
    QLabel *m_cpuTitleLabel;
    QLabel *m_cpuTempValueLabel;
    QLabel *m_gpuTitleLabel;
    QLabel *m_gpuTempValueLabel;
    QLabel *m_mbTitleLabel;
    QLabel *m_mbTempValueLabel;

    // Mapas para os labels de armazenamento criados dinamicamente
    QMap<QString, QLabel*> m_storageTitleLabels;
    QMap<QString, QLabel*> m_storageValueLabels;
    QVBoxLayout *m_storagePageLayout;

    // --- CORREÇÃO: Assinatura da função atualizada ---
    QWidget* createTemperatureRow(const QString &title, QLabel* &titleLabel, QLabel* &valueLabel);
    void updateButtonStyles(QPushButton *activeButton, QList<QPushButton*> &buttonGroup);
};
#endif // MAINWINDOW_H
