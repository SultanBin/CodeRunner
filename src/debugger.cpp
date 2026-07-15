#include "debugger.h"
#include "miparser.h"
#include <QDebug>
#include <QProcess>
#include <QRegularExpression>

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
    
    // Start GDB in MI mode
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
        debugProcess->write("-gdb-exit\n");
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
        debugProcess->write("-exec-next\n");
    }
}

void Debugger::stepInto()
{
    if (debuggingActive && debugProcess) {
        debugProcess->write("-exec-step\n");
    }
}

void Debugger::stepOut()
{
    if (debuggingActive && debugProcess) {
        debugProcess->write("-exec-finish\n");
    }
}

void Debugger::continueExecution()
{
    if (debuggingActive && debugProcess) {
        debugProcess->write("-exec-continue\n");
    }
}

void Debugger::toggleBreakpoint(const QString &filename, int lineNumber)
{
    QPair<QString, int> breakpoint = {filename, lineNumber};
    
    if (breakpoints.contains(breakpoint)) {
        breakpoints.remove(breakpoint);
        if (debuggingActive && debugProcess) {
            debugProcess->write(("-break-delete " + QString::number(lineNumber) + "\n").toUtf8());
        }
    } else {
        breakpoints.insert(breakpoint);
        if (debuggingActive && debugProcess) {
            debugProcess->write(("-break-insert " + filename + ":" + QString::number(lineNumber) + "\n").toUtf8());
        }
    }
}

void Debugger::clearBreakpoints()
{
    breakpoints.clear();
    if (debuggingActive && debugProcess) {
        debugProcess->write("-break-delete --all\n");
    }
}

void Debugger::inspectVariable(const QString &variableName)
{
    if (debuggingActive && debugProcess) {
        // Use MI evaluate expression
        debugProcess->write(("-data-evaluate-expression " + variableName + "\n").toUtf8());
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
    parseDebuggerOutput(error);
    emit outputReceived(error);
}

void Debugger::parseDebuggerOutput(const QString &output)
{
    if (output.isEmpty()) return;

    MIParser parser;
    QStringList lines = output.split('\n');

    for (const QString &rawLine : lines) {
        QString line = rawLine.trimmed();
        if (line.isEmpty()) continue;

        MIParser::ParsedResult pr = parser.parse(line);

        // Process MI events
        for (const DebugEvent &ev : pr.events) {
            if (ev.type == DebugEvent::Stopped) {
                // Try to use MI 'frame' result if present
                if (pr.results.contains("frame")) {
                    QVariant frameVar = pr.results.value("frame");
                    if (frameVar.canConvert<QVariantMap>()) {
                        QVariantMap fm = frameVar.toMap();
                        QString file = fm.value("file").toString();
                        int lineNo = fm.value("line").toInt();
                        if (!file.isEmpty() && lineNo > 0) {
                            currentFile = file;
                            currentLine = lineNo;
                            emit breakpointHit(currentFile, currentLine);
                            continue;
                        }
                    }
                }

                // Fallback heuristics
                QRegularExpression frameRe(R"(([\w\./\\-]+):(\d+))");
                auto m = frameRe.match(line);
                if (m.hasMatch()) {
                    currentFile = m.captured(1);
                    currentLine = m.captured(2).toInt();
                    emit breakpointHit(currentFile, currentLine);
                    continue;
                }
            }
        }

        // If MI parse contains results, look for value/name pairs
        if (!pr.results.isEmpty()) {
            if (pr.results.contains("frame")) {
                QVariantMap fm = pr.results.value("frame").toMap();
                QString file = fm.value("file").toString();
                int lineNo = fm.value("line").toInt();
                if (!file.isEmpty() && lineNo > 0) {
                    currentFile = file;
                    currentLine = lineNo;
                    emit breakpointHit(currentFile, currentLine);
                    continue;
                }
            }

            if (pr.results.contains("value")) {
                // Try to extract a variable name from the line
                QRegularExpression varRe(R"(name=\"?([a-zA-Z_][a-zA-Z0-9_]*)\"?.*value=\"?([^,}]+)\"?)");
                auto vm = varRe.match(line);
                if (vm.hasMatch()) {
                    QString name = vm.captured(1);
                    QString value = vm.captured(2);
                    variableValues.insert(name, value);
                    emit variableUpdated(name, value);
                    continue;
                }
            }
        }

        // Generic prints: match 'name = value'
        QRegularExpression printRe(R"(^\s*([A-Za-z_][A-Za-z0-9_]*)\s*=\s*(.+)$)");
        auto pm = printRe.match(line);
        if (pm.hasMatch()) {
            QString name = pm.captured(1);
            QString val = pm.captured(2).trimmed();
            variableValues.insert(name, val);
            emit variableUpdated(name, val);
            continue;
        }

        // Otherwise, ignore or forward as generic output
        // qDebug() << "Debugger output (unparsed):" << line;
    }
}
