#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QString>
#include <QList>
#include <QVariant>

struct GameData {
    int id = -1;
    QString executableName;
    QString displayName;
    QString coverPath;
    QString user_display_name;
    double allTimeAverageFps = 0.0;
};

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    static DatabaseManager& instance();

    // ALTERADO: Função mais robusta para adicionar ou atualizar
    bool addOrUpdateGame(const QString& executableName, const QString& displayName, const QString& coverPath);
    int getGameId(const QString& executableName);
    bool isGameKnown(const QString& executableName);
    GameData getGameData(const QString& executableName);
    QList<GameData> getGamesByMostRecent();
    bool updateGameCover(int gameId, const QString& coverPath); // Renomeado para clareza
    bool removeGame(const QString& executableName);
    bool addGameSession(int gameId, qint64 startTime, qint64 endTime, double averageFps);
    bool setManualGameName(const QString& executableName, const QString& newName);

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    void initDatabase();

    QSqlDatabase m_db;
};

#endif // DATABASEMANAGER_H
