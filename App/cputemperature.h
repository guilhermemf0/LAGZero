#ifndef CPUTEMPERATURE_H
#define CPUTEMPERATURE_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QProcess>
#include <QMap>
#include <QString>

// --- CORREÇÃO: Estrutura para armazenar nome, temperatura e tipo de disco ---
struct HardwareInfo {
    QString name = "N/D";
    double temperature = -1.0;
    QString driveType = ""; // Pode ser "SSD" ou "HD"
};

class TemperatureWorker : public QObject
{
    Q_OBJECT
public:
    TemperatureWorker();
public slots:
    void process();
signals:
    void temperaturesUpdated(const QMap<QString, HardwareInfo> &deviceInfos);
private slots:
    void readTemperature();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
private:
    QTimer *m_timer = nullptr;
    QProcess *m_process = nullptr;
};

class CpuTemperature : public QObject
{
    Q_OBJECT
public:
    explicit CpuTemperature(QObject *parent = nullptr);
    ~CpuTemperature();
signals:
    void temperaturesUpdated(const QMap<QString, HardwareInfo> &deviceInfos);
private:
    QThread workerThread;
};

#endif // CPUTEMPERATURE_H
