#include "apimanager.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

ApiManager::ApiManager(QObject *parent) : QObject(parent)
{
    m_netManager = new QNetworkAccessManager(this);
    // A sua chave de API
    m_apiKey = "3fe08a2d9e17d31f5fa710961754f938";
}

void ApiManager::searchGame(const QString& executableName)
{
    // Lógica de busca aprimorada
    QString searchTerm = QFileInfo(executableName).baseName();
    searchTerm.replace('_', ' ');
    searchTerm.replace('-', ' ');
    searchTerm.remove(" game", Qt::CaseInsensitive);
    searchTerm.remove("64", Qt::CaseInsensitive);
    searchTerm = searchTerm.trimmed();

    QUrl url("https://www.steamgriddb.com/api/v2/search/autocomplete/" + searchTerm);
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", ("Bearer " + m_apiKey).toUtf8());

    QNetworkReply *reply = m_netManager->get(request);
    reply->setProperty("executableName", executableName);
    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        onSearchReply(reply);
    });
}

void ApiManager::onSearchReply(QNetworkReply *reply)
{
    ApiGameResult result;
    result.executableName = reply->property("executableName").toString();

    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        if (doc.object()["success"].toBool() && !doc.object()["data"].toArray().isEmpty()) {
            QJsonObject game = doc.object()["data"].toArray()[0].toObject();
            int gameId = game["id"].toInt();
            result.name = game["name"].toString();

            QUrl posterUrl("https://www.steamgriddb.com/api/v2/posters/game/" + QString::number(gameId));
            QNetworkRequest request(posterUrl);
            request.setRawHeader("Authorization", ("Bearer " + m_apiKey).toUtf8());

            QNetworkReply *posterReply = m_netManager->get(request);
            connect(posterReply, &QNetworkReply::finished, this, [this, posterReply, result]() mutable {
                if (posterReply->error() == QNetworkReply::NoError) {
                    QJsonDocument posterDoc = QJsonDocument::fromJson(posterReply->readAll());
                    if (posterDoc.object()["success"].toBool() && !posterDoc.object()["data"].toArray().isEmpty()) {
                        result.success = true;
                        QJsonArray posters = posterDoc.object()["data"].toArray();
                        QString bestUrl = posters[0].toObject()["url"].toString(); // Fallback

                        for(const QJsonValue &val : posters) {
                            if(val.toObject()["language"].toString() == "pt") {
                                bestUrl = val.toObject()["url"].toString();
                                break;
                            }
                        }
                        result.coverUrl = bestUrl;
                    }
                }
                emit searchFinished(result);
                posterReply->deleteLater();
            });

        } else {
            emit searchFinished(result);
        }
    } else {
        qWarning() << "API search error:" << reply->errorString();
        emit searchFinished(result);
    }

    reply->deleteLater();
}


void ApiManager::downloadImage(const QUrl& url, const QString& savePath)
{
    m_currentImagePath = savePath;
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "image/webp,image/png,image/*,*/*;q=0.8");

    QNetworkReply *reply = m_netManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        onImageReply(reply);
    });
}

void ApiManager::onImageReply(QNetworkReply *reply)
{
    QUrl originalUrl = reply->url();

    if (reply->error() == QNetworkReply::NoError) {
        QFile file(m_currentImagePath);
        QDir dir = QFileInfo(m_currentImagePath).dir();
        if (!dir.exists()) {
            dir.mkpath(".");
        }

        if (file.open(QIODevice::WriteOnly)) {
            file.write(reply->readAll());
            file.close();
            emit imageDownloaded(m_currentImagePath, originalUrl);
        } else {
            qWarning() << "Could not save downloaded image to" << m_currentImagePath;
        }
    } else {
        qWarning() << "Image download error:" << reply->errorString();
    }

    reply->deleteLater();
}
