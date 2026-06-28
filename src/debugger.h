#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <QObject>
#include <QString>
#include <QProcess>
#include <QMap>
#include <QSet>

class Debugger : public QObject
{
    Q_OBJECT

public:
    explicit Debugger(QObject *parent = nullptr);
    ~Debugger();
    
    // Debug operations
    void startDebug(const QString &executable);
    void stopDebug();
    void stepOver();
    void stepInto();
    void stepOut();
    void continueExecution();
    
    // Breakpoint management
    void toggleBreakpoint(const QString &filename, int lineNumber);
    void clearBreakpoints();
    
    // Variable inspection
    void inspectVariable(const QString &variableName);
    QString getVariableValue(const QString &variableName) const;
    
    // State queries
    bool isDebugging() const { return debuggingActive; }
    int getCurrentLine() const { return currentLine; }
    QString getCurrentFile() const { return currentFile; }
    
signals:
    void debugStarted();
    void debugFinished();
    void breakpointHit(const QString &filename, int lineNumber);
    void variableUpdated(const QString &name, const QString &value);
    void outputReceived(const QString &output);
    void debugError(const QString &error);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();

private:
    void parseDebuggerOutput(const QString &output);
    
    std::unique_ptr<QProcess> debugProcess;
    bool debuggingActive;
    QString currentFile;
    int currentLine;
    QMap<QString, QString> variableValues;
    QSet<QPair<QString, int>> breakpoints; // filename, line number pairs
};

#endif // DEBUGGER_H
