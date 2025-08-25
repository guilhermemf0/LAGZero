#ifndef HARDWAREMONITOR_H
#define HARDWAREMONITOR_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QProcess>
#include <QMap>
#include <QString>

// A struct permanece a mesma, pois descreve bem a informação
struct HardwareInfo {
    QString name = "N/D";
    double temperature = -1.0;
    QString driveType = "";
};

// --- RENOMEADO: De TemperatureWorker para HardwareWorker ---
class HardwareWorker : public QObject
{
    Q_OBJECT
public:
    HardwareWorker();
public slots:
    void process();
signals:
    // --- RENOMEADO: De temperaturesUpdated para hardwareUpdated ---
    void hardwareUpdated(const QMap<QString, HardwareInfo> &deviceInfos);
private slots:
    void readHardwareData();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
private:
    QTimer *m_timer = nullptr;
    QProcess *m_process = nullptr;
};

// --- RENOMEADO: De CpuTemperature para HardwareMonitor ---
class HardwareMonitor : public QObject
{
    Q_OBJECT
public:
    explicit HardwareMonitor(QObject *parent = nullptr);
    ~HardwareMonitor();
signals:
    // --- RENOMEADO: De temperaturesUpdated para hardwareUpdated ---
    void hardwareUpdated(const QMap<QString, HardwareInfo> &deviceInfos);
private:
    QThread workerThread;
};

#endif // HARDWAREMONITOR_H
