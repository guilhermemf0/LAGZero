#ifndef HARDWAREMONITOR_H
#define HARDWAREMONITOR_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QProcess>
#include <QMap>
#include <QString>

struct HardwareInfo {
    QString name = "N/D";
    double temperature = -1.0;
    QString driveType = "";
};

class HardwareWorker : public QObject
{
    Q_OBJECT
public:
    HardwareWorker();
    ~HardwareWorker(); // Adicionado destrutor

public slots:
    void process();

signals:
    void hardwareUpdated(const QMap<QString, HardwareInfo> &deviceInfos);
    void helperMissing();

private slots:
    void readHardwareData();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QTimer *m_timer = nullptr;
    QProcess *m_process = nullptr;
};

class HardwareMonitor : public QObject
{
    Q_OBJECT
public:
    explicit HardwareMonitor(QObject *parent = nullptr);
    ~HardwareMonitor();

signals:
    void hardwareUpdated(const QMap<QString, HardwareInfo> &deviceInfos);
    void helperMissing();

private:
    QThread workerThread;
};

#endif // HARDWAREMONITOR_H
