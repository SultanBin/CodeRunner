#include "compiler.h"
#include <QProcess>
#include <QDebug>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QSysInfo>

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
    // TODO: Implement project-level compilation
    qDebug() << "Compiling project:" << projectPath;
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
