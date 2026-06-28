#ifndef ERRORPARSER_H
#define ERRORPARSER_H

#include <QString>
#include <QVector>
#include <QRegularExpression>

struct ParsedError
{
    QString filename;
    int lineNumber;
    int columnNumber;
    QString severity;   // "error", "warning", "note", "info"
    QString message;
    QString code;       // Error code if available
    QString fullLine;   // Original line from compiler output
};

class ErrorParser
{
public:
    ErrorParser();
    
    // Parse compiler output
    QVector<ParsedError> parseOutput(const QString &output);
    
    // Parse individual lines
    ParsedError parseLine(const QString &line);
    
    // Statistics
    int getErrorCount(const QString &output) const;
    int getWarningCount(const QString &output) const;
    
private:
    ParsedError parseGccFormat(const QString &line);
    ParsedError parseMsvcFormat(const QString &line);
    ParsedError parseClangFormat(const QString &line);
    ParsedError parseGenericFormat(const QString &line);
    
    // Regex patterns
    QRegularExpression gccPattern;
    QRegularExpression msvcPattern;
    QRegularExpression clangPattern;
};

#endif // ERRORPARSER_H
