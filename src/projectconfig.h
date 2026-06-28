#ifndef PROJECTCONFIG_H
#define PROJECTCONFIG_H

#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMap>

class ProjectConfig
{
public:
    ProjectConfig();
    explicit ProjectConfig(const QString &configFile);
    
    // File operations
    bool load(const QString &configFile);
    bool save(const QString &configFile);
    
    // Project properties
    QString getProjectName() const { return projectName; }
    void setProjectName(const QString &name) { projectName = name; }
    
    QString getProjectPath() const { return projectPath; }
    void setProjectPath(const QString &path) { projectPath = path; }
    
    // Compiler settings
    QString getCompiler() const { return compiler; }
    void setCompiler(const QString &comp) { compiler = comp; }
    
    QString getStandard() const { return standard; }
    void setStandard(const QString &std) { standard = std; }
    
    QString getOptimization() const { return optimization; }
    void setOptimization(const QString &opt) { optimization = opt; }
    
    bool getDebugInfo() const { return debugInfo; }
    void setDebugInfo(bool enable) { debugInfo = enable; }
    
    bool getWarnings() const { return warnings; }
    void setWarnings(bool enable) { warnings = enable; }
    
    // Paths
    QStringList getIncludePaths() const { return includePaths; }
    void setIncludePaths(const QStringList &paths) { includePaths = paths; }
    void addIncludePath(const QString &path) { includePaths.append(path); }
    void removeIncludePath(const QString &path) { includePaths.removeAll(path); }
    
    QStringList getLibraryPaths() const { return libraryPaths; }
    void setLibraryPaths(const QStringList &paths) { libraryPaths = paths; }
    void addLibraryPath(const QString &path) { libraryPaths.append(path); }
    void removeLibraryPath(const QString &path) { libraryPaths.removeAll(path); }
    
    QStringList getLibraries() const { return libraries; }
    void setLibraries(const QStringList &libs) { libraries = libs; }
    
    // Directories
    QString getSourceDir() const { return sourceDir; }
    void setSourceDir(const QString &dir) { sourceDir = dir; }
    
    QString getBuildDir() const { return buildDir; }
    void setBuildDir(const QString &dir) { buildDir = dir; }
    
    QString getBinDir() const { return binDir; }
    void setBinDir(const QString &dir) { binDir = dir; }
    
    // Custom flags
    QString getCustomFlags() const { return customFlags; }
    void setCustomFlags(const QString &flags) { customFlags = flags; }
    
    // Utilities
    bool isValid() const;
    void setDefaults();
    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);
    
private:
    QString projectName;
    QString projectPath;
    QString compiler;           // gcc, clang, msvc, mingw
    QString standard;           // c++11, c++14, c++17, c++20
    QString optimization;       // -O0, -O1, -O2, -O3, -Os
    bool debugInfo;
    bool warnings;
    QStringList includePaths;
    QStringList libraryPaths;
    QStringList libraries;
    QString sourceDir;
    QString buildDir;
    QString binDir;
    QString customFlags;
};

#endif // PROJECTCONFIG_H
