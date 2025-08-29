#include "steamappcache.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDir>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QDebug>
#include <QRegularExpression>

SteamAppCache& SteamAppCache::instance()
{
    static SteamAppCache instance;
    return instance;
}

SteamAppCache::SteamAppCache(QObject *parent) : QObject(parent)
{
    loadCache();
}

SteamAppCache::~SteamAppCache() {}

bool SteamAppCache::isCacheReady() const
{
    return m_isReady;
}

void SteamAppCache::loadCache()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString cachePath = path + "/steam_app_cache.json";
    QFile cacheFile(cachePath);

    QFileInfo fileInfo(cachePath);
    if (!cacheFile.exists() || fileInfo.lastModified().addDays(7) < QDateTime::currentDateTime()) {
        downloadAppList();
        return;
    }

    if (!cacheFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Não foi possível abrir o cache de apps da Steam. Tentando baixar novamente.";
        downloadAppList();
        return;
    }

    QByteArray data = cacheFile.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject root = doc.object();
    QJsonArray apps = root["applist"].toObject()["apps"].toArray();

    m_appList.reserve(apps.size());
    for (const QJsonValue& value : apps) {
        QJsonObject obj = value.toObject();
        m_appList.append({obj["appid"].toInt(), obj["name"].toString()});
    }

    m_isReady = true;
    emit cacheReady();
    qDebug() << m_appList.size() << "jogos carregados do cache local da Steam.";
}

void SteamAppCache::downloadAppList()
{
    qDebug() << "Baixando lista de aplicativos da Steam...";
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QUrl url("https://api.steampowered.com/ISteamApps/GetAppList/v2/");
    QNetworkRequest request(url);

    QNetworkReply *reply = manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        onAppListReply(reply);
    });
}

void SteamAppCache::onAppListReply(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        saveCache(data);
        loadCache();
    } else {
        qWarning() << "Falha ao baixar a lista de apps da Steam:" << reply->errorString();
    }
    reply->manager()->deleteLater();
    reply->deleteLater();
}

void SteamAppCache::saveCache(const QByteArray& data)
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(path);
    if (!dir.exists()) dir.mkpath(".");

    QString cachePath = path + "/steam_app_cache.json";
    QFile cacheFile(cachePath);
    if (cacheFile.open(QIODevice::WriteOnly)) {
        cacheFile.write(data);
        cacheFile.close();
        qDebug() << "Cache de apps da Steam salvo em:" << cachePath;
    } else {
        qWarning() << "Não foi possível salvar o cache de apps da Steam.";
    }
}

int SteamAppCache::findAppId(const QString& gameName) const
{
    if (!m_isReady) return 0;

    QString cleanGameName = gameName.toLower();
    cleanGameName.remove(QRegularExpression("[^a-z0-9]"));

    for (const auto& app : m_appList) {
        if (app.name.toLower() == gameName.toLower()) {
            return app.appId;
        }
    }

    for (const auto& app : m_appList) {
        QString cleanAppName = app.name.toLower();
        cleanAppName.remove(QRegularExpression("[^a-z0-9]"));
        // --- CORREÇÃO AQUI ---
        if (!cleanGameName.isEmpty() && cleanAppName.contains(cleanGameName)) {
            return app.appId;
        }
    }

    return 0;
}
