#ifndef COMPILER_H
#define COMPILER_H

#include <QObject>
#include <QString>
#include <QProcess>
#include <QStringList>

class Compiler : public QObject
{
    Q_OBJECT

public:
    enum class CompilerType
    {
        GCC,
        Clang,
        MinGW,
        MSVC,
        Unknown
    };
    
    explicit Compiler(QObject *parent = nullptr);
    ~Compiler();
    
    // Compiler operations
    void compileFile(const QString &sourceFile, const QString &outputFile);
    void compileProject(const QString &projectPath);
    void stop();
    
    // Compiler management
    void detectInstalledCompilers();
    void setActiveCompiler(CompilerType type);
    CompilerType getActiveCompiler() const { return activeCompiler; }
    
    // Configuration
    void setOptimizationLevel(const QString &level);
    void setIncludePaths(const QStringList &paths);
    void setLibraryPaths(const QStringList &paths);
    void addCompilerFlags(const QString &flags);
    
signals:
    void compilationStarted();
    void compilationFinished(bool success, const QString &output);
    void compilationError(const QString &error);
    void compilationWarning(const QString &warning);
    void outputReceived(const QString &output);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();

private:
    QString getCompilerPath();
    QStringList buildCompileCommand(const QString &sourceFile, const QString &outputFile);
    bool isCompilerAvailable(const QString &compiler);
    
    CompilerType activeCompiler;
    std::unique_ptr<QProcess> process;
    QString compilationOutput;
    QString optimizationLevel;
    QStringList includePaths;
    QStringList libraryPaths;
    QString compilerFlags;
};

#endif // COMPILER_H
