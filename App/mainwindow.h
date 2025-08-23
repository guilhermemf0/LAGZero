#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "cputemperature.h" // Inclua o header da nossa classe

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
    // Slot agora recebe as três temperaturas
    void onTemperatureUpdated(double cpu, double motherboard, double gpu);

private:
    Ui::MainWindow *ui;
    CpuTemperature *m_cpuTempReader; // Ponteiro para o nosso leitor
};
#endif // MAINWINDOW_H
