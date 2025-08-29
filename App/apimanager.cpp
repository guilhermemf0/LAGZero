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
#include <algorithm>
#include <vector>
#include <QRegularExpression>

QString cleanStringForComparison(QString name) {
    name = name.toLower();
    name = name.split('(').first().trimmed();
    name = name.split('/').first().trimmed();
    name.remove(QRegularExpression(":|hd remaster|biohazard|®|™|goty|edition|remake"));
    name.remove(QRegularExpression("[^a-z0-9\\s]"));
    return name.trimmed().replace(QRegularExpression("\\s+"), " ");
}

int levenshteinDistance(const QString &s1, const QString &s2) {
    const int len1 = s1.size(), len2 = s2.size();
    std::vector<int> col(len2 + 1), prevCol(len2 + 1);
    for (int i = 0; i < prevCol.size(); i++) prevCol[i] = i;
    for (int i = 0; i < len1; i++) {
        col[0] = i + 1;
        for (int j = 0; j < len2; j++)
            col[j + 1] = std::min({ prevCol[j + 1] + 1, col[j] + 1, prevCol[j] + (s1[i] == s2[j] ? 0 : 1) });
        col.swap(prevCol);
    }
    return prevCol[len2];
}


ApiManager::ApiManager(QObject *parent) : QObject(parent)
{
    m_netManager = new QNetworkAccessManager(this);
}

void ApiManager::findGameInfo(const QString& executableName, const QString& displayName)
{
    int appId = SteamAppCache::instance().findAppId(displayName);

    if (appId > 0) {
        QString steamName;
        for (const auto& app : SteamAppCache::instance().getAppList()) {
            if (app.appId == appId) {
                steamName = app.name;
                break;
            }
        }

        int distance = levenshteinDistance(cleanStringForComparison(steamName), cleanStringForComparison(displayName));

        if (distance < (displayName.length() / 3)) {
            qDebug() << "[ApiManager] Jogo encontrado no cache da Steam e validado:" << displayName << "->" << steamName << "AppID:" << appId;
            searchById(executableName, appId, displayName);
            return;
        } else {
            qDebug() << "[ApiManager] Jogo encontrado no cache da Steam, mas rejeitado por baixa similaridade. Distancia:" << distance;
        }
    }

    qDebug() << "[ApiManager] Jogo não encontrado/validado no cache. Buscando por nome na API:" << displayName;
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
    QString searchTerm = gameName.split('/').first().trimmed();

    if (searchTerm.isEmpty()) {
        searchTerm = QFileInfo(executableName).baseName();
    }

    QString encodedSearchTerm = QUrl::toPercentEncoding(searchTerm);
    QUrl url("https://www.steamgriddb.com/api/v2/search/autocomplete/" + encodedSearchTerm);

    QNetworkRequest request(url);
    request.setRawHeader("Authorization", ("Bearer " + m_apiKey).toUtf8());
    request.setRawHeader("User-Agent", "LagZeroApp/1.0");

    QNetworkReply *reply = m_netManager->get(request);
    reply->setProperty("executableName", executableName);
    reply->setProperty("originalGameName", gameName);
    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        onNameSearchReply(reply);
    });
}

void ApiManager::onNameSearchReply(QNetworkReply *reply)
{
    ApiGameResult result;
    result.executableName = reply->property("executableName").toString();
    QString originalGameName = reply->property("originalGameName").toString();

    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        if (doc.object()["success"].toBool() && !doc.object()["data"].toArray().isEmpty()) {

            QJsonArray resultsArray = doc.object()["data"].toArray();
            QJsonObject bestMatchObject;
            int lowestDistance = 1000;

            // --- LÓGICA DE COMPARAÇÃO ATUALIZADA ---
            // 1. Limpamos o nome original da janela para ter uma base de comparação limpa.
            QString cleanOriginalName = cleanStringForComparison(originalGameName);

            for (const QJsonValue& val : resultsArray) {
                QJsonObject currentObject = val.toObject();
                QString currentApiName = currentObject["name"].toString();

                // 2. Limpamos também o nome vindo da API com a MESMA função.
                QString cleanApiName = cleanStringForComparison(currentApiName);

                // 3. Comparamos as duas strings limpas.
                int distance = levenshteinDistance(cleanApiName, cleanOriginalName);

                if (distance < lowestDistance) {
                    lowestDistance = distance;
                    bestMatchObject = currentObject;
                }
            }

            int gameId = bestMatchObject["id"].toInt();
            result.name = bestMatchObject["name"].toString();

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
            QJsonArray allGrids = doc.object()["data"].toArray();

            QString bestUrl;
            int maxVotes = -1;

            QString fallbackUrl = allGrids[0].toObject()["url"].toString();


            for (const QJsonValue &val : allGrids) {
                QJsonObject gridObj = val.toObject();
                int height = gridObj["height"].toInt();
                int width = gridObj["width"].toInt();

                if (height > width) {
                    int upvotes = gridObj["upvotes"].toInt();
                    if (upvotes > maxVotes) {
                        maxVotes = upvotes;
                        bestUrl = gridObj["url"].toString();
                    }
                }
            }

            result.coverUrl = bestUrl.isEmpty() ? fallbackUrl : bestUrl;

            QList<QJsonObject> portraitGrids;
            for (const QJsonValue &val : allGrids) {
                QJsonObject gridObj = val.toObject();
                if (gridObj["height"].toInt() > gridObj["width"].toInt()) {
                    portraitGrids.append(gridObj);
                }
            }
            if(!portraitGrids.isEmpty()){
                emit gridListAvailable(result.executableName, portraitGrids);
            }
        }
    } else {
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
