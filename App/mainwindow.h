#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QString>
#include <QColor>

// Forward declarations
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class QLabel;
class CpuTemperature;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void onTemperaturesUpdated(const QMap<QString, double> &temps);
private:
    Ui::MainWindow *ui;
    CpuTemperature *m_tempReader;
    QMap<QString, QLabel*> m_storageLabels;

    void setLabelText(QLabel* label, const QString& prefix, double temperature, bool isCpu = false);
    QColor getCpuColor(double temperature);
};
#endif // MAINWINDOW_H
