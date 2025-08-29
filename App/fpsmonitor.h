#ifndef FPSMONITOR_H
#define FPSMONITOR_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QString>
#include <QMap>
#include <QVector>
#include <QDateTime>
#include <cstdint>

#ifdef Q_OS_WIN
#include <windows.h>
#include "RTSSSharedMemory.h"
#endif

struct GameSessionInfo {
    QString appName;
    QString exeName;
    qint64 startTime;
    QVector<int> fpsSamples;
};

class FpsWorker : public QObject
{
    Q_OBJECT
public:
    FpsWorker();
    ~FpsWorker();

public slots:
    void process();

signals:
    void rtssStatusUpdated(bool found, const QString& installPath);
    void gameSessionStarted(const QString& exeName, const QString& windowTitle, uint32_t processId);
    void gameSessionEnded(uint32_t processId, const QString& exeName, double averageFps);
    void activeGameFpsUpdate(uint32_t processId, int currentFps);

private slots:
    void readFps();

private:
    bool isRtssRunning();
    QString getRtssInstallPath();
    QString getWindowTitleByProcessId(DWORD processId);

    QTimer *m_timer = nullptr;
    QMap<uint32_t, GameSessionInfo> m_activeSessions;
};

class FpsMonitor : public QObject
{
    Q_OBJECT
public:
    explicit FpsMonitor(QObject *parent = nullptr);
    ~FpsMonitor();

signals:
    void rtssStatusUpdated(bool found, const QString& installPath);
    void gameSessionStarted(const QString& exeName, const QString& windowTitle, uint32_t processId);
    void gameSessionEnded(uint32_t processId, const QString& exeName, double averageFps);
    void activeGameFpsUpdate(uint32_t processId, int currentFps);

private:
    QThread workerThread;
};

#endif // FPSMONITOR_H
