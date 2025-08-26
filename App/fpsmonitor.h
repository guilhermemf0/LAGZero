#ifndef FPSMONITOR_H
#define FPSMONITOR_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QString>
#include <QMap>
#include <QVector>
#include <QDateTime>
#include <cstdint> // CORREÇÃO: Incluído para uint32_t

#ifdef Q_OS_WIN
#include <windows.h>
#include "RTSSSharedMemory.h"
#endif

// Estrutura para guardar informações de uma sessão de jogo ativa
struct GameSessionInfo {
    QString appName;
    QString exeName;
    qint64 startTime;
    QVector<int> fpsSamples;
};

// --- Worker que lê os dados do RTSS ---
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

    // Sinais para o gerenciamento de sessões de jogo
    // CORREÇÃO: Trocado DWORD por uint32_t
    void gameSessionStarted(const QString& exeName, uint32_t processId);
    void gameSessionEnded(uint32_t processId, const QString& exeName, double averageFps);
    void activeGameFpsUpdate(uint32_t processId, int currentFps);

private slots:
    void readFps();

private:
    bool isRtssRunning();
    QString getRtssInstallPath();

    QTimer *m_timer = nullptr;
    // CORREÇÃO: Trocado DWORD por uint32_t
    QMap<uint32_t, GameSessionInfo> m_activeSessions;
};

// --- Classe principal que controla o worker ---
class FpsMonitor : public QObject
{
    Q_OBJECT
public:
    explicit FpsMonitor(QObject *parent = nullptr);
    ~FpsMonitor();

signals:
    // Sinais encaminhados do worker
    void rtssStatusUpdated(bool found, const QString& installPath);
    // CORREÇÃO: Trocado DWORD por uint32_t
    void gameSessionStarted(const QString& exeName, uint32_t processId);
    void gameSessionEnded(uint32_t processId, const QString& exeName, double averageFps);
    void activeGameFpsUpdate(uint32_t processId, int currentFps);

private:
    QThread workerThread;
};

#endif // FPSMONITOR_H
