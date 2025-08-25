#ifndef FPSMONITOR_H
#define FPSMONITOR_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QString>
#include <QRegularExpression> // Para parsing com regex
#include <QSettings> // Adicionado para QSettings

#ifdef Q_OS_WIN
#include <windows.h> // Necessário para APIs de Windows
#include "RTSSSharedMemory.h" // AGORA INCLUÍMOS O ARQUIVO OFICIAL DO RTSS
#endif

// --- Worker que lê os dados do RTSS ---
class FpsWorker : public QObject
{
    Q_OBJECT
public:
    FpsWorker();
    ~FpsWorker(); // Destrutor para liberar recursos

public:
    void process(); // Função para iniciar o processamento do worker

signals:
    // Sinal agora emite o FPS e o nome do app para a thread principal
    void fpsUpdated(int fps, const QString& appName);
    // NOVO: Sinal para informar o status do RTSS (encontrado/não encontrado e caminho de instalação)
    void rtssStatusUpdated(bool found, const QString& installPath);

private:
    void readFps(); // Função privada para ler os dados de FPS
    // NOVO: Funções para verificar o status do RTSS
    bool isRtssRunning();
    QString getRtssInstallPath();

    QTimer *m_timer = nullptr; // Timer para chamar readFps periodicamente
    void* m_pMapFile = nullptr; // Ponteiro para a memória mapeada
    void* m_hMapFile = nullptr; // Handle para o objeto de mapeamento de arquivo
};

// --- Classe principal que controla o worker ---
class FpsMonitor : public QObject
{
    Q_OBJECT
public:
    explicit FpsMonitor(QObject *parent = nullptr);
    ~FpsMonitor(); // Destrutor para garantir que a thread seja finalizada

signals:
    // Sinal encaminhado do worker com ambos os valores de FPS e nome do app
    void fpsUpdated(int fps, const QString& appName);
    // NOVO: Sinal encaminhado do worker para informar o status do RTSS
    void rtssStatusUpdated(bool found, const QString& installPath);

private:
    QThread workerThread; // Thread dedicada para o worker
};

#endif // FPSMONITOR_H
