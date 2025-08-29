#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager dmInstance;
    return dmInstance;
}

DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent)
{
    initDatabase();
}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

void DatabaseManager::initDatabase()
{
    m_db = QSqlDatabase::addDatabase("QSQLITE", "game_connection");
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(path);
    if (!dir.exists())
        dir.mkpath(".");
    m_db.setDatabaseName(path + "/lagzero_gamedb.sqlite");

    if (!m_db.open()) {
        qWarning() << "Error: connection with database failed:" << m_db.lastError();
        return;
    }

    QSqlQuery query(m_db);
    if (!query.exec("CREATE TABLE IF NOT EXISTS games ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "executable_name TEXT UNIQUE NOT NULL, "
                    "display_name TEXT, "
                    "cover_path TEXT)")) {
        qWarning() << "Failed to create table 'games':" << query.lastError();
    }

    if (!query.exec("CREATE TABLE IF NOT EXISTS game_sessions ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "game_id INTEGER, "
                    "start_time INTEGER, "
                    "end_time INTEGER, "
                    "average_fps REAL, "
                    "FOREIGN KEY(game_id) REFERENCES games(id) ON DELETE CASCADE)")) {
        qWarning() << "Failed to create table 'game_sessions':" << query.lastError();
    }
}

// NOVO: Implementação da função "INSERT OR UPDATE"
bool DatabaseManager::addOrUpdateGame(const QString& executableName, const QString& displayName, const QString& coverPath)
{
    QSqlQuery query(m_db);
    // Esta sintaxe do SQLite insere uma nova linha. Se a chave 'executable_name' já existir,
    // ela executa a ação de UPDATE.
    query.prepare("INSERT INTO games (executable_name, display_name, cover_path) "
                  "VALUES (:exe, :display, :cover) "
                  "ON CONFLICT(executable_name) DO UPDATE SET "
                  "display_name = excluded.display_name, "
                  "cover_path = IIF(excluded.cover_path = '', games.cover_path, excluded.cover_path)");
    query.bindValue(":exe", executableName);
    query.bindValue(":display", displayName);
    query.bindValue(":cover", coverPath);

    if (!query.exec()) {
        qWarning() << "Failed to add or update game:" << query.lastError();
        return false;
    }
    return true;
}

int DatabaseManager::getGameId(const QString& executableName)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT id FROM games WHERE executable_name = :exe");
    query.bindValue(":exe", executableName);
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return -1;
}

bool DatabaseManager::isGameKnown(const QString& executableName)
{
    return getGameId(executableName) != -1;
}

GameData DatabaseManager::getGameData(const QString& executableName)
{
    GameData data;
    data.executableName = executableName;

    QSqlQuery query(m_db);
    query.prepare("SELECT id, display_name, cover_path FROM games WHERE executable_name = :exe");
    query.bindValue(":exe", executableName);

    if (query.exec() && query.next()) {
        data.id = query.value(0).toInt();
        data.displayName = query.value(1).toString();
        data.coverPath = query.value(2).toString();

        QSqlQuery avgQuery(m_db);
        avgQuery.prepare("SELECT AVG(average_fps) FROM game_sessions WHERE game_id = :id");
        avgQuery.bindValue(":id", data.id);
        if (avgQuery.exec() && avgQuery.next()) {
            data.allTimeAverageFps = avgQuery.value(0).toDouble();
        }
    }
    return data;
}

QList<GameData> DatabaseManager::getGamesByMostRecent()
{
    QList<GameData> games;
    QSqlQuery query(m_db);
    query.prepare("SELECT g.executable_name FROM games g "
                  "LEFT JOIN (SELECT game_id, MAX(end_time) as max_end_time FROM game_sessions GROUP BY game_id) s "
                  "ON g.id = s.game_id "
                  "ORDER BY s.max_end_time DESC");

    if (query.exec()) {
        while (query.next()) {
            games.append(getGameData(query.value(0).toString()));
        }
    } else {
        qWarning() << "Failed to get games by most recent:" << query.lastError();
    }
    return games;
}

// ALTERADO: Renomeado para clareza
bool DatabaseManager::updateGameCover(int gameId, const QString& coverPath)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE games SET cover_path = :cover WHERE id = :id");
    query.bindValue(":cover", coverPath);
    query.bindValue(":id", gameId);
    if (!query.exec()) {
        qWarning() << "Failed to update game cover:" << query.lastError();
        return false;
    }
    return true;
}

bool DatabaseManager::removeGame(const QString& executableName)
{
    int gameId = getGameId(executableName);
    if (gameId == -1) return false;

    QSqlQuery query(m_db);
    query.prepare("DELETE FROM games WHERE id = :id");
    query.bindValue(":id", gameId);

    if (!query.exec()) {
        qWarning() << "Failed to remove game:" << query.lastError();
        return false;
    }
    return true;
}


bool DatabaseManager::addGameSession(int gameId, qint64 startTime, qint64 endTime, double averageFps)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO game_sessions (game_id, start_time, end_time, average_fps) "
                  "VALUES (:id, :start, :end, :avg_fps)");
    query.bindValue(":id", gameId);
    query.bindValue(":start", startTime);
    query.bindValue(":end", endTime);
    query.bindValue(":avg_fps", averageFps);
    if (!query.exec()) {
        qWarning() << "Failed to add game session:" << query.lastError();
        return false;
    }
    return true;
}
