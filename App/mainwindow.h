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
    // Slot para receber a nova temperatura e atualizar a UI
    void onTemperatureUpdated(double temperature);

private:
    Ui::MainWindow *ui;
    CpuTemperature *m_cpuTempReader; // Ponteiro para o nosso leitor
};
#endif // MAINWINDOW_H
