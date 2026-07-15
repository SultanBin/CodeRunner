#include "compiler.h"
#include <QProcess>
#include <QDebug>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QSysInfo>
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include "projectconfig.h"

Compiler::Compiler(QObject *parent)
    : QObject(parent), activeCompiler(CompilerType::GCC), optimizationLevel("-O2")
{
    process = std::make_unique<QProcess>();
    connect(process.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &Compiler::onProcessFinished);
    connect(process.get(), QOverload<QProcess::ProcessError>::of(&QProcess::error),
            this, &Compiler::onProcessError);
    connect(process.get(), &QProcess::readyReadStandardOutput,
            this, &Compiler::onReadyReadStandardOutput);
    connect(process.get(), &QProcess::readyReadStandardError,
            this, &Compiler::onReadyReadStandardError);
    
    detectInstalledCompilers();
}

Compiler::~Compiler()
{
    if (process && process->state() == QProcess::Running) {
        process->kill();
        process->waitForFinished();
    }
}

void Compiler::compileFile(const QString &sourceFile, const QString &outputFile)
{
    if (process->state() == QProcess::Running) {
        qWarning() << "Compilation already in progress";
        return;
    }
    
    compilationOutput.clear();
    
    QStringList arguments = buildCompileCommand(sourceFile, outputFile);
    QString compiler = getCompilerPath();
    
    if (compiler.isEmpty()) {
        emit compilationError("Compiler not found");
        return;
    }
    
    qDebug() << "Compiling with:" << compiler << arguments;
    
    emit compilationStarted();
    process->start(compiler, arguments);
}

void Compiler::compileProject(const QString &projectPath)
{
    if (process->state() == QProcess::Running) {
        qWarning() << "Compilation already in progress";
        return;
    }

    compilationOutput.clear();

    // Load project config
    ProjectConfig cfg;
    QFileInfo pathInfo(projectPath);
    QString configFile;
    if (pathInfo.isDir()) {
        // Look for common project config names
        QStringList candidates = {"project.json", "project.conf", "coderunner.json", "projectconfig.json"};
        for (const QString &name : candidates) {
            QString candidate = QDir(projectPath).filePath(name);
            if (QFileInfo::exists(candidate)) {
                configFile = candidate;
                break;
            }
        }
        if (!configFile.isEmpty()) {
            if (!cfg.load(configFile)) {
                qWarning() << "Failed to load project config from" << configFile;
                // continue with defaults
            }
        } else {
            // No config found: set defaults with projectPath
            cfg.setProjectPath(projectPath);
            cfg.setProjectName(QFileInfo(projectPath).fileName());
        }
    } else if (pathInfo.isFile()) {
        // If given a file, try to load it as config
        if (!cfg.load(projectPath)) {
            qWarning() << "Failed to load project config from" << projectPath;
            // Fallback: treat parent dir as project
            cfg.setProjectPath(pathInfo.absolutePath());
            cfg.setProjectName(pathInfo.baseName());
        }
    } else {
        // Path doesn't exist; error out
        emit compilationError("Project path does not exist: " + projectPath);
        emit compilationFinished(false, compilationOutput);
        return;
    }

    // Resolve directories
    QString basePath = cfg.getProjectPath().isEmpty() ? projectPath : cfg.getProjectPath();
    QDir baseDir(basePath);
    QString sourceDir = baseDir.filePath(cfg.getSourceDir());
    QString buildDir = baseDir.filePath(cfg.getBuildDir());
    QString binDir = baseDir.filePath(cfg.getBinDir());

    // Collect source files
    QStringList sources;
    QDirIterator it(sourceDir, QStringList() << "*.cpp" << "*.cc" << "*.cxx" << "*.c",
                    QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        sources << it.next();
    }

    if (sources.isEmpty()) {
        emit compilationError("No source files found in " + sourceDir);
        emit compilationFinished(false, compilationOutput);
        return;
    }

    // Ensure build/bin directories exist
    QDir().mkpath(buildDir);
    QDir().mkpath(binDir);

    // Output file
    QString outputFile = QDir(binDir).filePath(cfg.getProjectName());
#ifdef Q_OS_WIN
    outputFile += ".exe";
#endif

    // Build arguments
    QStringList arguments;
    for (const QString &s : sources) arguments << s;

    arguments << "-o" << outputFile;

    // Optimization and standard
    if (!cfg.getOptimization().isEmpty()) arguments << cfg.getOptimization();
    if (!cfg.getStandard().isEmpty()) arguments << ("-std=" + cfg.getStandard());

    // Custom flags
    if (!cfg.getCustomFlags().isEmpty()) {
        QStringList custom = cfg.getCustomFlags().split(' ', Qt::SkipEmptyParts);
        arguments << custom;
    }

    // Include paths
    for (const QString &inc : cfg.getIncludePaths()) {
        arguments << ("-I" + inc);
    }

    // Library paths and libs
    for (const QString &libPath : cfg.getLibraryPaths()) {
        arguments << ("-L" + libPath);
    }
    for (const QString &lib : cfg.getLibraries()) {
        arguments << ("-l" + lib);
    }

    // Merge in any configured global include/library paths for this Compiler instance
    for (const QString &inc : includePaths) arguments << ("-I" + inc);
    for (const QString &libPath : libraryPaths) arguments << ("-L" + libPath);
    if (!compilerFlags.isEmpty()) {
        arguments << compilerFlags.split(' ', Qt::SkipEmptyParts);
    }

    QString compilerPath = getCompilerPath();
    if (compilerPath.isEmpty()) {
        emit compilationError("Compiler not found");
        emit compilationFinished(false, compilationOutput);
        return;
    }

    qDebug() << "Compiling project with:" << compilerPath << arguments;
    emit compilationStarted();
    process->start(compilerPath, arguments);
}

void Compiler::stop()
{
    if (process && process->state() == QProcess::Running) {
        process->terminate();
        if (!process->waitForFinished(3000)) {
            process->kill();
            process->waitForFinished();
        }
    }
}

void Compiler::detectInstalledCompilers()
{
    // Try to detect available compilers
    if (isCompilerAvailable("g++")) {
        qDebug() << "GCC found";
        activeCompiler = CompilerType::GCC;
    } else if (isCompilerAvailable("clang++")) {
        qDebug() << "Clang found";
        activeCompiler = CompilerType::Clang;
    }
#ifdef Q_OS_WIN
    else if (isCompilerAvailable("cl.exe")) {
        qDebug() << "MSVC found";
        activeCompiler = CompilerType::MSVC;
    }
#endif
}

void Compiler::setActiveCompiler(Compiler::CompilerType type)
{
    activeCompiler = type;
}

void Compiler::setOptimizationLevel(const QString &level)
{
    optimizationLevel = level;
}

void Compiler::setIncludePaths(const QStringList &paths)
{
    includePaths = paths;
}

void Compiler::setLibraryPaths(const QStringList &paths)
{
    libraryPaths = paths;
}

void Compiler::addCompilerFlags(const QString &flags)
{
    compilerFlags = flags;
}

void Compiler::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    bool success = exitStatus == QProcess::NormalExit && exitCode == 0;
    emit compilationFinished(success, compilationOutput);
}

void Compiler::onProcessError(QProcess::ProcessError error)
{
    QString errorMsg;
    switch (error) {
    case QProcess::FailedToStart:
        errorMsg = "Failed to start compiler";
        break;
    case QProcess::Crashed:
        errorMsg = "Compiler crashed";
        break;
    case QProcess::Timedout:
        errorMsg = "Compilation timed out";
        break;
    default:
        errorMsg = "Unknown error";
        break;
    }
    emit compilationError(errorMsg);
    emit compilationFinished(false, compilationOutput);
}

void Compiler::onReadyReadStandardOutput()
{
    compilationOutput += QString::fromUtf8(process->readAllStandardOutput());
    emit outputReceived(compilationOutput);
}

void Compiler::onReadyReadStandardError()
{
    QString error = QString::fromUtf8(process->readAllStandardError());
    compilationOutput += error;
    emit compilationWarning(error);
}

QString Compiler::getCompilerPath()
{
    switch (activeCompiler) {
    case CompilerType::GCC:
        return "g++";
    case CompilerType::Clang:
        return "clang++";
    case CompilerType::MinGW:
        return "g++";
    case CompilerType::MSVC:
        return "cl.exe";
    default:
        return "g++";
    }
}

QStringList Compiler::buildCompileCommand(const QString &sourceFile, const QString &outputFile)
{
    QStringList arguments;
    
    arguments << sourceFile;
    arguments << "-o" << outputFile;
    arguments << optimizationLevel;
    arguments << "-std=c++17";
    arguments << "-Wall" << "-Wextra";
    
    // Add include paths
    for (const QString &path : includePaths) {
        arguments << ("-I" + path);
    }
    
    // Add library paths
    for (const QString &path : libraryPaths) {
        arguments << ("-L" + path);
    }
    
    // Add custom flags
    if (!compilerFlags.isEmpty()) {
        arguments << compilerFlags;
    }
    
    return arguments;
}

bool Compiler::isCompilerAvailable(const QString &compiler)
{
    QProcess proc;
    proc.start(compiler, QStringList() << "--version");
    bool result = proc.waitForFinished(1000);
    return result && proc.exitCode() == 0;
}
