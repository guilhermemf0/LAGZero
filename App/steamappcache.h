#ifndef STEAMAPPCACHE_H
#define STEAMAPPCACHE_H

#include <QObject>
#include <QList>
#include <QString>
#include <QNetworkReply>
#include <QDateTime>

struct SteamApp {
    int appId;
    QString name;
};

class SteamAppCache : public QObject
{
    Q_OBJECT
public:
    static SteamAppCache& instance();

    int findAppId(const QString& gameName) const;
    bool isCacheReady() const;

signals:
    void cacheReady();

private slots:
    void onAppListReply(QNetworkReply *reply);

private:
    explicit SteamAppCache(QObject *parent = nullptr);
    ~SteamAppCache();
    SteamAppCache(const SteamAppCache&) = delete;
    SteamAppCache& operator=(const SteamAppCache&) = delete;

    void loadCache();
    void downloadAppList();
    void saveCache(const QByteArray& data);

    QList<SteamApp> m_appList;
    bool m_isReady = false;
};

#endif // STEAMAPPCACHE_H
