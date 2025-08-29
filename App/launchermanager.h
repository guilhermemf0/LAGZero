#ifndef LAUNCHERMANAGER_H
#define LAUNCHERMANAGER_H

#include <QObject>
#include <QString>
#include <cstdint>

class LauncherManager : public QObject
{
    Q_OBJECT
public:
    static LauncherManager& instance();

    QString findGameDisplayName(const QString& executablePath, uint32_t processId);

private:
    explicit LauncherManager(QObject *parent = nullptr);
    ~LauncherManager();
    LauncherManager(const LauncherManager&) = delete;
    LauncherManager& operator=(const LauncherManager&) = delete;

    QString findUwpGameDisplayName(uint32_t processId);
    QString findEpicGameDisplayName(const QString& executablePath);
    QString findGogGameDisplayName(const QString& executablePath);
    QString findEaGameDisplayName(const QString& executablePath);
    QString findBattlenetGameDisplayName(const QString& executablePath);
    QString findUbisoftGameDisplayName(const QString& executablePath);
};

#endif // LAUNCHERMANAGER_H
