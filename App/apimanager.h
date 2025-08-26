#ifndef APIMANAGER_H
#define APIMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkReply>

// Estrutura para o resultado da busca na API
struct ApiGameResult {
    bool success = false;
    QString name;
    QString coverUrl;
    QString executableName; // Para manter o contexto
};

class ApiManager : public QObject
{
    Q_OBJECT
public:
    explicit ApiManager(QObject *parent = nullptr);
    void searchGame(const QString& executableName);
    void downloadImage(const QUrl& url, const QString& savePath);

signals:
    void searchFinished(const ApiGameResult& result);
    void imageDownloaded(const QString& localPath, const QUrl& originalUrl);

private slots:
    void onSearchReply(QNetworkReply *reply);
    void onImageReply(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_netManager;
    // IMPORTANTE: Substitua pela sua chave de API da SteamGridDB
    QString m_apiKey = "3fe08a2d9e17d31f5fa710961754f938";
    QString m_currentImagePath;
};

#endif // APIMANAGER_H
