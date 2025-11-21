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
#include <QFileInfo>
#include <QSslSocket>

// --- IMPORTANTE: SUBSTITUA ISSO PELA SUA CHAVE DA STEAM ---
const QString STEAM_API_KEY = "BEF5AD89A9945E2AD93CFF99A4E269AA";
// ----------------------------------------------------------

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
        qWarning() << "Não foi possível abrir o cache local. Tentando baixar.";
        downloadAppList();
        return;
    }

    QByteArray data = cacheFile.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isNull()) {
        QJsonObject root = doc.object();

        // A API v1 (IStoreService) retorna dentro de "response"
        // A API v2 (ISteamApps) retorna dentro de "applist"
        QJsonArray apps;
        if (root.contains("response")) {
            apps = root["response"].toObject()["apps"].toArray();
        } else if (root.contains("applist")) {
            apps = root["applist"].toObject()["apps"].toArray();
        }

        m_appList.reserve(apps.size());
        for (const QJsonValue& value : apps) {
            QJsonObject obj = value.toObject();
            // Na v1 o ID pode vir como "id" ou "appid", garantimos a leitura
            int appId = obj.contains("appid") ? obj["appid"].toInt() : obj["id"].toInt();
            QString name = obj["name"].toString();

            if (appId > 0 && !name.isEmpty()) {
                m_appList.append({appId, name});
            }
        }
        qDebug() << m_appList.size() << "jogos carregados do cache local.";
    }

    m_isReady = true;
    emit cacheReady();
}

void SteamAppCache::downloadAppList()
{
    if (STEAM_API_KEY == "COLE_SUA_CHAVE_AQUI") {
        qWarning() << "ERRO: Você precisa colocar sua API KEY no arquivo steamappcache.cpp!";
        m_isReady = true;
        emit cacheReady();
        return;
    }

    qDebug() << "Baixando lista de aplicativos da Steam (v1 IStoreService)...";
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    // URL da v1 com os parâmetros corretos
    QString urlStr = QString("https://api.steampowered.com/IStoreService/GetAppList/v1/?key=%1&max_results=50000&include_games=true&include_dlc=false&include_software=false")
                         .arg(STEAM_API_KEY);

    QUrl url(urlStr);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "LagZeroApp/1.0");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply *reply = manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        onAppListReply(reply);
    });
}

void SteamAppCache::onAppListReply(QNetworkReply *reply)
{
    QByteArray data = reply->readAll();

    if (reply->error() == QNetworkReply::NoError && !data.isEmpty()) {
        qDebug() << "Download da Steam concluído com sucesso!";
        saveCache(data);
        loadCache();
    } else {
        qWarning() << "ERRO Steam API:" << reply->errorString();
        qWarning() << "Código HTTP:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (!data.isEmpty()) qWarning() << "CONTEÚDO DO ERRO:" << data;

        m_isReady = true;
        emit cacheReady();
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
    }
}

const QList<SteamApp>& SteamAppCache::getAppList() const
{
    return m_appList;
}

int SteamAppCache::findAppId(const QString& gameName) const
{
    if (!m_isReady || m_appList.isEmpty()) return 0;

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
        if (!cleanGameName.isEmpty() && cleanAppName.contains(cleanGameName)) {
            return app.appId;
        }
    }
    return 0;
}
