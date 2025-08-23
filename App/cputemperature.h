#ifndef CPUTEMPERATURE_H
#define CPUTEMPERATURE_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QProcess>

// --- Classe Worker ---
class TemperatureWorker : public QObject
{
    Q_OBJECT

public:
    TemperatureWorker();

public slots:
    void process();

signals:
    void temperatureUpdated(double temperature);

private slots:
    void readTemperature();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QTimer *m_timer = nullptr;
    QProcess *m_process = nullptr;
};


// --- Classe Principal (Controladora) ---
class CpuTemperature : public QObject
{
    Q_OBJECT
public:
    explicit CpuTemperature(QObject *parent = nullptr);
    ~CpuTemperature();

signals:
    void temperatureUpdated(double temperature);

private:
    QThread workerThread;
};

#endif // CPUTEMPERATURE_H
