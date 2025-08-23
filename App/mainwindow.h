#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QString>

// Forward declarations
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    // Slot agora recebe o mapa de temperaturas
    void onTemperaturesUpdated(const QMap<QString, double> &temps);
private:
    Ui::MainWindow *ui;
    class CpuTemperature *m_tempReader;
    // Mapa para guardar os labels dos discos que criamos
    QMap<QString, QLabel*> m_storageLabels;
};
#endif // MAINWINDOW_H
