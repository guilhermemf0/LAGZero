#ifndef FPSMONITOR_H
#define FPSMONITOR_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QString>
#include <QRegularExpression> // Para parsing com regex

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

public: // MUDADO: process() não é mais um slot do Qt
    void process(); // Função para iniciar o processamento do worker

signals:
    // Sinal agora emite o FPS e o nome do app para a thread principal
    void fpsUpdated(int fps, const QString& appName);

private:
    void readFps(); // Função privada para ler os dados de FPS
    // Função para ler o FPS diretamente das entradas de aplicativo do RTSS.
    // Não é mais um slot, então não precisa de `public slots:`
    // void UpdateOSD(LPCSTR lpText, QString &appName, int &fpsValue); // Removido, lógica em readFps
    // void ReleaseOSD(); // Removido, lógica no destrutor ou não mais necessária

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

private:
    QThread workerThread; // Thread dedicada para o worker
};

#endif // FPSMONITOR_H
