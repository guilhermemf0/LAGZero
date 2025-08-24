#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QMap>
#include <QColor>

// Enumeração para os ícones das abas
enum IconIndex {
    OverviewIcon = 0,
    CpuIcon,
    GpuIcon,
    MotherboardIcon,
    StorageIcon,
    SettingsIcon
};

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class CpuTemperature;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Slot para atualizar as temperaturas
    void onTemperaturesUpdated(const QMap<QString, double> &temps);
    // Slot para lidar com os cliques dos botões de navegação
    void onNavigationButtonClicked();

private:
    Ui::MainWindow *ui;
    CpuTemperature *m_tempReader;
    QMap<QString, QLabel*> m_storageLabels;

    // Widgets para as abas principais
    QStackedWidget *m_mainStackedWidget;
    QWidget *m_overviewPage;
    QWidget *m_temperaturePage;
    QWidget *m_settingsPage;

    // Widgets para as sub-abas de temperatura
    QStackedWidget *m_tempSubStackedWidget;
    QWidget *m_cpuPage;
    QWidget *m_gpuPage;
    QWidget *m_motherboardPage;
    QWidget *m_storagePage;

    // Funções para estilizar e atualizar os rótulos de temperatura
    QColor getTemperatureColor(double temperature);
    void setLabelText(QLabel* label, const QString& prefix, double temperature);

    // Funções legadas que podem ser removidas mais tarde se não forem usadas
    QColor getCpuColor(double temperature);
};
#endif // MAINWINDOW_H
