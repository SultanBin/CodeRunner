#include "projectconfig.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include <QDir>

ProjectConfig::ProjectConfig()
    : debugInfo(false), warnings(true)
{
    setDefaults();
}

ProjectConfig::ProjectConfig(const QString &configFile)
    : debugInfo(false), warnings(true)
{
    setDefaults();
    load(configFile);
}

bool ProjectConfig::load(const QString &configFile)
{
    QFile file(configFile);
    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "Cannot open config file" << configFile;
        return false;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (!doc.isObject()) {
        qWarning() << "Invalid JSON in config file";
        return false;
    }
    
    fromJson(doc.object());
    return true;
}

bool ProjectConfig::save(const QString &configFile)
{
    QFile file(configFile);
    if (!file.open(QFile::WriteOnly)) {
        qWarning() << "Cannot write config file" << configFile;
        return false;
    }
    
    QJsonDocument doc(toJson());
    file.write(doc.toJson());
    file.close();
    
    return true;
}

bool ProjectConfig::isValid() const
{
    return !projectName.isEmpty() && !projectPath.isEmpty();
}

void ProjectConfig::setDefaults()
{
    projectName = "NewProject";
    projectPath = QDir::currentPath();
    compiler = "gcc";
    standard = "c++17";
    optimization = "-O2";
    debugInfo = true;
    warnings = true;
    sourceDir = "src";
    buildDir = "build";
    binDir = "bin";
    customFlags = "-Wall -Wextra";
}

QJsonObject ProjectConfig::toJson() const
{
    QJsonObject json;
    json["project"] = projectName;
    json["path"] = projectPath;
    json["compiler"] = compiler;
    json["standard"] = standard;
    json["optimization"] = optimization;
    json["debugInfo"] = debugInfo;
    json["warnings"] = warnings;
    json["customFlags"] = customFlags;
    json["sourceDir"] = sourceDir;
    json["buildDir"] = buildDir;
    json["binDir"] = binDir;
    
    // Arrays
    QJsonArray includePaths;
    for (const QString &path : this->includePaths) {
        includePaths.append(path);
    }
    json["includePaths"] = includePaths;
    
    QJsonArray libraryPaths;
    for (const QString &path : this->libraryPaths) {
        libraryPaths.append(path);
    }
    json["libraryPaths"] = libraryPaths;
    
    QJsonArray libraries;
    for (const QString &lib : this->libraries) {
        libraries.append(lib);
    }
    json["libraries"] = libraries;
    
    return json;
}

void ProjectConfig::fromJson(const QJsonObject &json)
{
    projectName = json["project"].toString("NewProject");
    projectPath = json["path"].toString(QDir::currentPath());
    compiler = json["compiler"].toString("gcc");
    standard = json["standard"].toString("c++17");
    optimization = json["optimization"].toString("-O2");
    debugInfo = json["debugInfo"].toBool(true);
    warnings = json["warnings"].toBool(true);
    customFlags = json["customFlags"].toString("-Wall -Wextra");
    sourceDir = json["sourceDir"].toString("src");
    buildDir = json["buildDir"].toString("build");
    binDir = json["binDir"].toString("bin");
    
    // Arrays
    includePaths.clear();
    for (const QJsonValue &val : json["includePaths"].toArray()) {
        includePaths.append(val.toString());
    }
    
    libraryPaths.clear();
    for (const QJsonValue &val : json["libraryPaths"].toArray()) {
        libraryPaths.append(val.toString());
    }
    
    libraries.clear();
    for (const QJsonValue &val : json["libraries"].toArray()) {
        libraries.append(val.toString());
    }
}
