#include "errorparser.h"
#include <QDebug>

ErrorParser::ErrorParser()
{
    // GCC/Clang: file:line:col: error/warning: message
    gccPattern = QRegularExpression(
        "^([^:]+):(\\d+):(\\d+):\\s*(error|warning|note):\\s*(.*)$"
    );
    
    // MSVC: file(line,col): error C####: message
    msvcPattern = QRegularExpression(
        "^([^(]+)\\((\\d+)(?:,(\\d+))?\\):\\s*(error|warning|note)\\s*([A-Z]\\d+)?:?\\s*(.*)$"
    );
    
    // Clang: Similar to GCC but with different color codes
    clangPattern = gccPattern;  // Same format as GCC
}

QVector<ParsedError> ErrorParser::parseOutput(const QString &output)
{
    QVector<ParsedError> errors;
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    
    for (const QString &line : lines) {
        ParsedError error = parseLine(line);
        if (!error.filename.isEmpty()) {
            errors.append(error);
        }
    }
    
    return errors;
}

ParseError ErrorParser::parseLine(const QString &line)
{
    // Try each parser in order
    ParsedError error = parseGccFormat(line);
    if (!error.filename.isEmpty()) return error;
    
    error = parseMsvcFormat(line);
    if (!error.filename.isEmpty()) return error;
    
    error = parseClangFormat(line);
    if (!error.filename.isEmpty()) return error;
    
    error = parseGenericFormat(line);
    return error;
}

int ErrorParser::getErrorCount(const QString &output) const
{
    return output.count(QRegularExpression("error", QRegularExpression::CaseInsensitiveOption));
}

int ErrorParser::getWarningCount(const QString &output) const
{
    return output.count(QRegularExpression("warning", QRegularExpression::CaseInsensitiveOption));
}

ParsedError ErrorParser::parseGccFormat(const QString &line)
{
    ParsedError error;
    error.fullLine = line;
    
    // Format: file:line:col: error/warning/note: message
    QRegularExpressionMatch match = gccPattern.match(line);
    
    if (match.hasMatch()) {
        error.filename = match.captured(1);
        error.lineNumber = match.captured(2).toInt();
        error.columnNumber = match.captured(3).toInt();
        error.severity = match.captured(4);
        error.message = match.captured(5);
        
        // Extract error code if present
        QRegularExpression codePattern("[-\\w]+");
        QRegularExpressionMatch codeMatch = codePattern.match(error.message);
        if (codeMatch.hasMatch()) {
            error.code = codeMatch.captured(0);
        }
    }
    
    return error;
}

ParsedError ErrorParser::parseMsvcFormat(const QString &line)
{
    ParsedError error;
    error.fullLine = line;
    
    // Format: file(line[,col]): error/warning [code]: message
    QRegularExpressionMatch match = msvcPattern.match(line);
    
    if (match.hasMatch()) {
        error.filename = match.captured(1);
        error.lineNumber = match.captured(2).toInt();
        error.columnNumber = match.captured(3).isEmpty() ? 1 : match.captured(3).toInt();
        error.severity = match.captured(4);
        error.code = match.captured(5);
        error.message = match.captured(6);
    }
    
    return error;
}

ParsedError ErrorParser::parseClangFormat(const QString &line)
{
    // Clang uses the same format as GCC
    return parseGccFormat(line);
}

ParsedError ErrorParser::parseGenericFormat(const QString &line)
{
    ParsedError error;
    error.fullLine = line;
    
    // Generic format: file line number message
    if (line.contains("error", Qt::CaseInsensitive)) {
        error.severity = "error";
    } else if (line.contains("warning", Qt::CaseInsensitive)) {
        error.severity = "warning";
    } else {
        error.severity = "info";
    }
    
    return error;
}
