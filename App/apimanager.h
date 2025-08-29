#ifndef APIMANAGER_H
#define APIMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkReply>

struct ApiGameResult {
    bool success = false;
    QString name;
    QString coverUrl;
    QString executableName;
};

class ApiManager : public QObject
{
    Q_OBJECT
public:
    explicit ApiManager(QObject *parent = nullptr);

    void findGameInfo(const QString& executableName, const QString& displayName);
    void downloadImage(const QUrl& url, const QString& savePath);

signals:
    void searchFinished(const ApiGameResult& result);
    void imageDownloaded(const QString& localPath, const QUrl& originalUrl);

private slots:
    void onNameSearchReply(QNetworkReply *reply);
    void onGridSearchReply(QNetworkReply *reply);
    void onImageReply(QNetworkReply *reply);

private:
    void searchByName(const QString& executableName, const QString& gameName);
    void searchById(const QString& executableName, int steamAppId, const QString& gameName);

    QNetworkAccessManager *m_netManager;
    QString m_apiKey = "93b1c0bae25de75ec9029f18314ddea2";
    QString m_currentImagePath;
};

#endif // APIMANAGER_H
