#include "debugger.h"
#include <QDebug>
#include <QProcess>

Debugger::Debugger(QObject *parent)
    : QObject(parent), debuggingActive(false), currentLine(0)
{
    debugProcess = std::make_unique<QProcess>();
    connect(debugProcess.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &Debugger::onProcessFinished);
    connect(debugProcess.get(), &QProcess::readyReadStandardOutput,
            this, &Debugger::onReadyReadStandardOutput);
    connect(debugProcess.get(), &QProcess::readyReadStandardError,
            this, &Debugger::onReadyReadStandardError);
}

Debugger::~Debugger()
{
    stopDebug();
}

void Debugger::startDebug(const QString &executable)
{
    if (debuggingActive) {
        qWarning() << "Debugging already in progress";
        return;
    }
    
    // Start GDB or LLDB
#ifdef Q_OS_WIN
    debugProcess->start("gdb", QStringList() << "--interpreter=mi2" << executable);
#else
    debugProcess->start("gdb", QStringList() << "--interpreter=mi2" << executable);
#endif
    
    debuggingActive = true;
    emit debugStarted();
}

void Debugger::stopDebug()
{
    if (debugProcess && debugProcess->state() == QProcess::Running) {
        debugProcess->write("quit\n");
        if (!debugProcess->waitForFinished(3000)) {
            debugProcess->kill();
            debugProcess->waitForFinished();
        }
    }
    debuggingActive = false;
    breakpoints.clear();
    variableValues.clear();
    emit debugFinished();
}

void Debugger::stepOver()
{
    if (debuggingActive && debugProcess) {
        debugProcess->write("next\n");
    }
}

void Debugger::stepInto()
{
    if (debuggingActive && debugProcess) {
        debugProcess->write("step\n");
    }
}

void Debugger::stepOut()
{
    if (debuggingActive && debugProcess) {
        debugProcess->write("finish\n");
    }
}

void Debugger::continueExecution()
{
    if (debuggingActive && debugProcess) {
        debugProcess->write("continue\n");
    }
}

void Debugger::toggleBreakpoint(const QString &filename, int lineNumber)
{
    QPair<QString, int> breakpoint = {filename, lineNumber};
    
    if (breakpoints.contains(breakpoint)) {
        breakpoints.remove(breakpoint);
        if (debuggingActive && debugProcess) {
            debugProcess->write(("clear " + filename + ":" + QString::number(lineNumber) + "\n").toUtf8());
        }
    } else {
        breakpoints.insert(breakpoint);
        if (debuggingActive && debugProcess) {
            debugProcess->write(("break " + filename + ":" + QString::number(lineNumber) + "\n").toUtf8());
        }
    }
}

void Debugger::clearBreakpoints()
{
    breakpoints.clear();
    if (debuggingActive && debugProcess) {
        debugProcess->write("delete\n");
    }
}

void Debugger::inspectVariable(const QString &variableName)
{
    if (debuggingActive && debugProcess) {
        debugProcess->write(("print " + variableName + "\n").toUtf8());
    }
}

QString Debugger::getVariableValue(const QString &variableName) const
{
    return variableValues.value(variableName, "<unknown>");
}

void Debugger::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
    debuggingActive = false;
    emit debugFinished();
}

void Debugger::onReadyReadStandardOutput()
{
    QString output = QString::fromUtf8(debugProcess->readAllStandardOutput());
    parseDebuggerOutput(output);
    emit outputReceived(output);
}

void Debugger::onReadyReadStandardError()
{
    QString error = QString::fromUtf8(debugProcess->readAllStandardError());
    emit outputReceived(error);
}

void Debugger::parseDebuggerOutput(const QString &output)
{
    // Parse GDB/LLDB output and update variables
    Q_UNUSED(output);
    // TODO: Implement proper parsing
}
