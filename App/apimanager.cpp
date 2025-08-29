#include "apimanager.h"
#include "steamappcache.h"
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
}

void ApiManager::findGameInfo(const QString& executableName, const QString& displayName)
{
    int appId = SteamAppCache::instance().findAppId(displayName);
    if (appId > 0) {
        qDebug() << "Jogo encontrado no cache da Steam:" << displayName << "-> AppID:" << appId;
        searchById(executableName, appId, displayName);
        return;
    }

    qDebug() << "Jogo não encontrado no cache. Buscando por nome na API:" << displayName;
    searchByName(executableName, displayName);
}

void ApiManager::searchById(const QString& executableName, int steamAppId, const QString& gameName)
{
    QUrl url("https://www.steamgriddb.com/api/v2/games/steam/" + QString::number(steamAppId));
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", ("Bearer " + m_apiKey).toUtf8());
    request.setRawHeader("User-Agent", "LagZeroApp/1.0");

    QNetworkReply *reply = m_netManager->get(request);
    reply->setProperty("executableName", executableName);
    reply->setProperty("gameName", gameName);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            if (doc.object()["success"].toBool()) {
                int gameId = doc.object()["data"].toObject()["id"].toInt();

                // --- CORREÇÃO APLICADA AQUI ---
                // Trocado "posters" por "grids", como você corretamente apontou na documentação.
                QUrl gridUrl("https://www.steamgriddb.com/api/v2/grids/game/" + QString::number(gameId));
                QNetworkRequest gridReq(gridUrl);
                gridReq.setRawHeader("Authorization", ("Bearer " + m_apiKey).toUtf8());
                gridReq.setRawHeader("User-Agent", "LagZeroApp/1.0");

                QNetworkReply *gridReply = m_netManager->get(gridReq);
                gridReply->setProperty("executableName", reply->property("executableName").toString());
                gridReply->setProperty("gameName", reply->property("gameName").toString());
                connect(gridReply, &QNetworkReply::finished, this, [this, gridReply]() {
                    onGridSearchReply(gridReply);
                });
            }
        } else {
            qWarning() << "API steam lookup error:" << reply->errorString();
        }
        reply->deleteLater();
    });
}

void ApiManager::searchByName(const QString& executableName, const QString& gameName)
{
    QString searchTerm = gameName;
    if (searchTerm.isEmpty()) {
        searchTerm = QFileInfo(executableName).baseName();
        searchTerm.replace('_', ' ');
        searchTerm.replace('-', ' ');
        searchTerm = searchTerm.trimmed();
    }

    QUrl url("https://www.steamgriddb.com/api/v2/search/autocomplete/" + searchTerm);
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", ("Bearer " + m_apiKey).toUtf8());
    request.setRawHeader("User-Agent", "LagZeroApp/1.0");

    QNetworkReply *reply = m_netManager->get(request);
    reply->setProperty("executableName", executableName);
    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        onNameSearchReply(reply);
    });
}

void ApiManager::onNameSearchReply(QNetworkReply *reply)
{
    ApiGameResult result;
    result.executableName = reply->property("executableName").toString();

    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        if (doc.object()["success"].toBool() && !doc.object()["data"].toArray().isEmpty()) {
            QJsonObject game = doc.object()["data"].toArray()[0].toObject();
            int gameId = game["id"].toInt();
            result.name = game["name"].toString();

            // --- CORREÇÃO APLICADA AQUI ---
            // Trocado "posters" por "grids".
            QUrl gridUrl("https://www.steamgriddb.com/api/v2/grids/game/" + QString::number(gameId));
            QNetworkRequest request(gridUrl);
            request.setRawHeader("Authorization", ("Bearer " + m_apiKey).toUtf8());
            request.setRawHeader("User-Agent", "LagZeroApp/1.0");

            QNetworkReply *gridReply = m_netManager->get(request);
            gridReply->setProperty("executableName", result.executableName);
            gridReply->setProperty("gameName", result.name);

            connect(gridReply, &QNetworkReply::finished, this, [this, gridReply]() {
                onGridSearchReply(gridReply);
            });
        } else {
            emit searchFinished(result);
        }
    } else {
        qWarning() << "API name search error:" << reply->errorString();
        emit searchFinished(result);
    }
    reply->deleteLater();
}

void ApiManager::onGridSearchReply(QNetworkReply *reply)
{
    ApiGameResult result;
    result.executableName = reply->property("executableName").toString();
    result.name = reply->property("gameName").toString();

    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        if (doc.object()["success"].toBool() && !doc.object()["data"].toArray().isEmpty()) {
            result.success = true;
            QJsonArray grids = doc.object()["data"].toArray();
            QString bestUrl = grids[0].toObject()["url"].toString();

            // Lógica para preferir capas em português (se houver)
            for(const QJsonValue &val : grids) {
                if(val.toObject()["language"].toString() == "pt") {
                    bestUrl = val.toObject()["url"].toString();
                    break;
                }
            }
            result.coverUrl = bestUrl;
        }
    } else {
        // Log corrigido para clareza
        qWarning() << "API grid search error:" << reply->errorString();
    }

    emit searchFinished(result);
    reply->deleteLater();
}

void ApiManager::downloadImage(const QUrl& url, const QString& savePath)
{
    m_currentImagePath = savePath;
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "image/webp,image/png,image/*,*/*;q=0.8");
    request.setRawHeader("User-Agent", "LagZeroApp/1.0");
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
        if (!dir.exists()) dir.mkpath(".");
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
