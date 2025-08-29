#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QSettings>
#include <QStandardPaths>
#include <QFileInfo>
#include <QTextStream>
#include <QProcess>
#include <QDir>
#include <QDirIterator>
#include "launchermanager.h"

#ifdef Q_OS_WIN
QString findInstallPathFromRegistry(const QString& registryPath) {
    QSettings registry(registryPath, QSettings::NativeFormat);
    QString installLocation = registry.value("InstallLocation").toString();
    if (!installLocation.isEmpty() && QDir(installLocation).exists()) {
        return QDir::fromNativeSeparators(installLocation);
    }
    return QString();
}
#endif

LauncherManager& LauncherManager::instance() { static LauncherManager instance; return instance; }
LauncherManager::LauncherManager(QObject *parent) : QObject(parent) {}
LauncherManager::~LauncherManager() {}

QString LauncherManager::findGameDisplayName(const QString& executablePath, uint32_t processId)
{
    QString gameName;

    // A detecção UWP é especial e precisa vir primeiro, usando o PID.
    gameName = findUwpGameDisplayName(processId);
    if (!gameName.isEmpty()) {
        qDebug() << "[LauncherManager] Jogo da Windows Store (UWP) encontrado:" << gameName;
        return gameName;
    }

    // O resto da cascata continua como antes, usando o caminho do executável.
    gameName = findEpicGameDisplayName(executablePath);
    if (!gameName.isEmpty()) { /* ... */ return gameName; }

    gameName = findGogGameDisplayName(executablePath);
    if (!gameName.isEmpty()) { /* ... */ return gameName; }

    gameName = findEaGameDisplayName(executablePath);
    if (!gameName.isEmpty()) { /* ... */ return gameName; }

    gameName = findBattlenetGameDisplayName(executablePath);
    if (!gameName.isEmpty()) { /* ... */ return gameName; }

    gameName = findUbisoftGameDisplayName(executablePath);
    if (!gameName.isEmpty()) { /* ... */ return gameName; }

    return QString();
}

QString LauncherManager::findUwpGameDisplayName(uint32_t processId)
{
#ifdef Q_OS_WIN
    if (processId == 0) return QString();

    QProcess process;
    // Este comando PowerShell pega o pacote AppX associado a um Process ID específico
    // e retorna suas propriedades como um objeto JSON. É rápido e confiável.
    QString command = QString("Get-AppxPackage -User (Get-Process -Id %1).User | Where-Object { $_.IsFramework -ne $true } | Select-Object DisplayName, Name | ConvertTo-Json").arg(processId);

    process.start("powershell.exe", {"-Command", command});
    process.waitForFinished(3000); // Espera no máximo 3 segundos

    QByteArray output = process.readAllStandardOutput();
    if (output.isEmpty()) return QString();

    QJsonDocument doc = QJsonDocument::fromJson(output);
    if (doc.isObject()) {
        // O nome de exibição geralmente está no campo "DisplayName"
        QString displayName = doc.object()["DisplayName"].toString();
        if (!displayName.isEmpty()){
            return displayName;
        }
        // Como fallback, usamos o campo "Name" se o DisplayName não estiver presente
        return doc.object()["Name"].toString();
    }
#else
    Q_UNUSED(processId);
#endif
    return QString();
}

QString LauncherManager::findEpicGameDisplayName(const QString& executablePath)
{
    QString programDataPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    QDir manifestDir(programDataPath + "/Epic/EpicGamesLauncher/Data/Manifests");

    if (!manifestDir.exists()) return QString();

    QDirIterator it(manifestDir.absolutePath(), {"*.item"}, QDir::Files);
    while (it.hasNext()) {
        QFile file(it.next());
        if (file.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            if (doc.isObject()) {
                QString displayName = doc.object()["DisplayName"].toString();
                QString installLocation = doc.object()["InstallLocation"].toString();
                if (!displayName.isEmpty() && !installLocation.isEmpty() &&
                    executablePath.startsWith(installLocation, Qt::CaseInsensitive)) {
                    return displayName;
                }
            }
        }
    }
    return QString();
}

QString LauncherManager::findGogGameDisplayName(const QString& executablePath)
{
    QString programDataPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    const QString dbPath = programDataPath + "/GOG.com/Galaxy/storage/galaxy-2.0.db";

    if (!QFile::exists(dbPath)) return QString();

    const QString connectionName = "gog_connection";
    QSqlDatabase gogDb = QSqlDatabase::database(connectionName, false); // Não abre se não existir
    if (!gogDb.isValid()) {
        gogDb = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    }

    gogDb.setDatabaseName(dbPath);

    if (!gogDb.open()) {
        qWarning() << "Não foi possível abrir o banco de dados do GOG Galaxy:" << gogDb.lastError();
        return QString();
    }

    QSqlQuery query(gogDb);
    query.prepare("SELECT gv.value FROM GamePieces gv "
                  "JOIN GamePieceValues gpv ON gv.id = gpv.gamePieceId "
                  "JOIN InstalledBaseProducts ibp ON gpv.productId = ibp.id "
                  "WHERE gv.gamePieceType = 'title' AND ibp.installPath = :path");
    query.bindValue(":path", QDir::toNativeSeparators(executablePath));

    QString gameTitle;
    if (query.exec() && query.next()) {
        gameTitle = query.value(0).toString();
    }

    gogDb.close();
    return gameTitle;
}

QString LauncherManager::findEaGameDisplayName(const QString& executablePath)
{
#ifdef Q_OS_WIN
    // A EA armazena os dados de jogos instalados no registro.
    QSettings registry("HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\EA Games", QSettings::NativeFormat);

    // Cada subchave é um jogo.
    for (const QString& gameKey : registry.childGroups()) {
        registry.beginGroup(gameKey);
        QString displayName = registry.value("DisplayName").toString();
        QString installDir = QDir::fromNativeSeparators(registry.value("Install Dir").toString()); // Nota: "Install Dir" com espaço
        registry.endGroup();

        if (!displayName.isEmpty() && !installDir.isEmpty() &&
            executablePath.startsWith(installDir, Qt::CaseInsensitive)) {
            return displayName;
        }
    }
#endif
    return QString();
}

QString LauncherManager::findBattlenetGameDisplayName(const QString& executablePath)
{
    QString programDataPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    QFile dbFile(programDataPath + "/Battle.net/Agent/product.db");

    if (!dbFile.open(QIODevice::ReadOnly | QIODevice::Text)) return QString();

    QTextStream in(&dbFile);
    // O formato do arquivo é proprietário, mas podemos extrair os dados.
    // Procuramos por blocos que contenham o caminho de instalação e o nome do produto.
    QString fileContent = in.readAll();
    QStringList products = fileContent.split("product_state"); // Divide o arquivo por jogos

    for (const QString& productBlock : products) {
        QString installDir;
        QString displayName;

        if (productBlock.contains("install_dir")) {
            // Extrai o caminho de instalação
            QString rawPath = productBlock.split("install_dir").last().split("\"").at(2);
            installDir = QDir::fromNativeSeparators(rawPath.replace("\\\\", "\\"));
        }

        if (productBlock.contains("product_name")) {
            // Extrai o nome do jogo
            displayName = productBlock.split("product_name").last().split("\"").at(2);
        }

        if (!installDir.isEmpty() && !displayName.isEmpty() &&
            executablePath.startsWith(installDir, Qt::CaseInsensitive)) {
            return displayName;
        }
    }
    return QString();
}


// Lógica para UBISOFT CONNECT - Usa o Registro do Windows para encontrar o launcher e depois aplica as 5 táticas de busca.
QString LauncherManager::findUbisoftGameDisplayName(const QString& executablePath)
{
    QString launcherPath;

#ifdef Q_OS_WIN
    launcherPath = findInstallPathFromRegistry("HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Ubisoft\\Launcher");
#endif
    if (launcherPath.isEmpty()) launcherPath = "C:/Program Files (x86)/Ubisoft/Ubisoft Game Launcher";
    if (!QDir(launcherPath).exists()) return QString();

    // 1. Primeira tentativa: procurar no local.yml (cache do Ubisoft Connect)
    QFile registryFile(launcherPath + "/cache/registry/local.yml");
    if (registryFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&registryFile);
        QString currentInstallDir, currentDisplayName;
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.trimmed().startsWith("InstallDir:")) {
                currentInstallDir = line.section(":", 1).trimmed().replace("\"", "");
                currentInstallDir = QDir::fromNativeSeparators(currentInstallDir);
            }
            if (line.trimmed().startsWith("DisplayName:")) {
                currentDisplayName = line.section(":", 1).trimmed().replace("\"", "");
            }
            if (!currentInstallDir.isEmpty() && !currentDisplayName.isEmpty()) {
                if (executablePath.startsWith(currentInstallDir, Qt::CaseInsensitive)) {
                    return currentDisplayName; // Encontrou, retorna imediatamente.
                }
                currentInstallDir.clear();
                currentDisplayName.clear();
            }
        }
    }

    QFileInfo exeInfo(executablePath);
    QString manifestPath = exeInfo.absolutePath() + "/../support/ubi/manifest.json";
    QFile manifestFile(QDir::cleanPath(manifestPath));
    if (manifestFile.exists() && manifestFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(manifestFile.readAll(), &error);
        if (error.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.contains("displayName")) {
                return obj["displayName"].toString();
            }
        }
    }
    QDir dataDir(launcherPath + "/data/");
    if (dataDir.exists()) {
        QStringList filters = {"*_installationInfo.json"};
        for (const QFileInfo& fileInfo : dataDir.entryInfoList(filters, QDir::Files)) {
            QFile file(fileInfo.absoluteFilePath());
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
                if (doc.isObject()) {
                    QJsonObject obj = doc.object();
                    QString installDir = QDir::fromNativeSeparators(obj["installDir"].toString());
                    QString displayName = obj["displayName"].toString();
                    if (!installDir.isEmpty() && !displayName.isEmpty() && executablePath.startsWith(installDir, Qt::CaseInsensitive)) {
                        return displayName; // Encontrou, retorna imediatamente.
                    }
                }
            }
        }
    }

#ifdef Q_OS_WIN
    QSettings registry("HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Ubisoft\\Launcher\\Installs", QSettings::NativeFormat);
    for (const QString& key : registry.childGroups()) {
        registry.beginGroup(key);
        QString installDir = QDir::fromNativeSeparators(registry.value("InstallDir").toString());
        QString displayName = registry.value("DisplayName").toString();
        registry.endGroup();
        if (!installDir.isEmpty() && !displayName.isEmpty() && executablePath.startsWith(installDir, Qt::CaseInsensitive)) {
            return displayName;
        }
    }
#endif
    QString folderName = QFileInfo(executablePath).dir().dirName();
    if (!folderName.isEmpty()){
        return folderName;
    }

    return QString();
}
