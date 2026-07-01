#ifndef MIPARSER_H
#define MIPARSER_H

#include <QString>
#include <QMap>
#include <QVector>
#include <QVariant>
#include "debugger_structs.h"

class MIParser
{
public:
    struct ParsedResult
    {
        QString token;
        QString resultClass;    // "done", "running", "connected", "error", "exit"
        QMap<QString, QVariant> results;
        QVector<DebugEvent> events;
    };
    
    MIParser();
    
    // Parse output lines
    ParsedResult parse(const QString &line);
    
    // Helper parsers
    Breakpoint parseBreakpoint(const QMap<QString, QVariant> &data);
    Variable parseVariable(const QMap<QString, QVariant> &data);
    StackFrame parseStackFrame(const QMap<QString, QVariant> &data);
    ThreadInfo parseThreadInfo(const QMap<QString, QVariant> &data);
    DebugEvent parseEvent(const QString &line);
    
private:
    QVariant parseValue(const QString &str);
    QMap<QString, QVariant> parseResult(const QString &str);
    QVector<QMap<QString, QVariant>> parseList(const QString &str);
    QString unescapeString(const QString &str);
    
    int findMatchingBracket(const QString &str, int start);
    int findMatchingBrace(const QString &str, int start);
};

#endif // MIPARSER_H
