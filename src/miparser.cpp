#include "miparser.h"
#include <QDebug>
#include <QRegularExpression>

MIParser::MIParser()
{
}

MIParser::ParsedResult MIParser::parse(const QString &line)
{
    ParsedResult result;
    result.resultClass = "error";
    
    if (line.isEmpty()) return result;
    
    // Parse token (optional number at start)
    QRegularExpression tokenPattern("^(\\d+)(.*)$");
    QRegularExpressionMatch match = tokenPattern.match(line);
    
    int offset = 0;
    if (match.hasMatch()) {
        result.token = match.captured(1);
        offset = result.token.length();
    }
    
    // Get result class
    if (line.length() > offset) {
        QChar resultChar = line.at(offset);
        switch (resultChar.toLatin1()) {
        case '~': // Console stream
        case '@': // Target stream
        case '&': // Log stream
        case '^': // Result
        case '*': // Exec async
        case '+': // Notify async
        case '=': // General async
            offset++;
            break;
        default:
            break;
        }
    }
    
    // Extract result class and data
    QString remaining = line.mid(offset).trimmed();
    QRegularExpression classPattern("^([a-z-]+)(.*)$");
    match = classPattern.match(remaining);
    
    if (match.hasMatch()) {
        result.resultClass = match.captured(1);
        QString data = match.captured(2).trimmed();
        
        if (!data.isEmpty() && data.at(0) == '{') {
            result.results = parseResult(data);
        }
    }
    
    return result;
}

Breakpoint MIParser::parseBreakpoint(const QMap<QString, QVariant> &data)
{
    Breakpoint bp;
    bp.id = data.value("number").toInt(-1);
    bp.file = data.value("file").toString();
    bp.line = data.value("line").toInt(-1);
    bp.enabled = data.value("enabled").toString() == "y";
    bp.condition = data.value("cond").toString();
    bp.hitCount = data.value("times").toInt(0);
    bp.type = Breakpoint::Line;
    return bp;
}

Variable MIParser::parseVariable(const QMap<QString, QVariant> &data)
{
    Variable var;
    var.name = data.value("name").toString();
    var.value = data.value("value").toString();
    var.type = data.value("type").toString();
    var.address = data.value("address").toString();
    var.numChildren = data.value("numchild").toInt(0);
    var.isExpandable = var.numChildren > 0;
    return var;
}

StackFrame MIParser::parseStackFrame(const QMap<QString, QVariant> &data)
{
    StackFrame frame;
    frame.level = data.value("level").toInt(0);
    frame.address = data.value("addr").toString();
    frame.function = data.value("func").toString();
    frame.file = data.value("file").toString();
    frame.line = data.value("line").toInt(0);
    frame.column = data.value("column").toInt(0);
    return frame;
}

ThreadInfo MIParser::parseThreadInfo(const QMap<QString, QVariant> &data)
{
    ThreadInfo thread;
    thread.id = data.value("id").toInt(0);
    thread.state = data.value("state").toString();
    thread.target_id = data.value("target-id").toString();
    thread.is_running = thread.state == "running";
    return thread;
}

DebugEvent MIParser::parseEvent(const QString &line)
{
    DebugEvent event;
    event.type = DebugEvent::Stopped;
    
    if (line.contains("stopped")) {
        event.type = DebugEvent::Stopped;
        if (line.contains("breakpoint-hit")) {
            event.reason = "breakpoint-hit";
        } else if (line.contains("end-stepping-range")) {
            event.reason = "end-stepping-range";
        }
    } else if (line.contains("running")) {
        event.type = DebugEvent::Running;
    } else if (line.contains("thread-created")) {
        event.type = DebugEvent::ThreadCreated;
    }
    
    return event;
}

QVariant MIParser::parseValue(const QString &str)
{
    // Simple value parser
    if (str.startsWith('"') && str.endsWith('"')) {
        return unescapeString(str.mid(1, str.length() - 2));
    } else if (str == "true") {
        return true;
    } else if (str == "false") {
        return false;
    } else if (str.startsWith('[')) {
        return parseList(str);
    } else if (str.startsWith('{')) {
        return parseResult(str);
    } else {
        return str;
    }
}

QMap<QString, QVariant> MIParser::parseResult(const QString &str)
{
    QMap<QString, QVariant> result;
    
    if (!str.startsWith('{') || !str.endsWith('}')) {
        return result;
    }
    
    QString content = str.mid(1, str.length() - 2);
    int i = 0;
    
    while (i < content.length()) {
        // Find key
        QRegularExpression keyPattern("^\\s*([a-zA-Z_][a-zA-Z0-9_-]*)\\s*=");
        QRegularExpressionMatch match = keyPattern.match(content.mid(i));
        
        if (!match.hasMatch()) break;
        
        QString key = match.captured(1);
        i += match.capturedEnd(0) - match.capturedStart(0);
        
        // Find value
        if (i < content.length()) {
            if (content.at(i) == '"') {
                // String value
                int end = i + 1;
                while (end < content.length() && content.at(end) != '"') {
                    if (content.at(end) == '\\') end++;
                    end++;
                }
                QString value = content.mid(i + 1, end - i - 1);
                result[key] = value;
                i = end + 1;
            } else if (content.at(i) == '{') {
                // Nested object
                int end = findMatchingBrace(content, i);
                result[key] = parseResult(content.mid(i, end - i + 1));
                i = end + 1;
            } else if (content.at(i) == '[') {
                // Array
                int end = findMatchingBracket(content, i);
                result[key] = parseList(content.mid(i, end - i + 1));
                i = end + 1;
            }
            
            // Skip comma
            while (i < content.length() && (content.at(i) == ',' || content.at(i).isSpace())) {
                i++;
            }
        }
    }
    
    return result;
}

QVector<QMap<QString, QVariant>> MIParser::parseList(const QString &str)
{
    QVector<QMap<QString, QVariant>> list;
    // TODO: Implement list parsing
    return list;
}

QString MIParser::unescapeString(const QString &str)
{
    QString result = str;
    result.replace("\\n", "\n");
    result.replace("\\t", "\t");
    result.replace("\\\"", "\"");
    result.replace("\\\\", "\\");
    return result;
}

int MIParser::findMatchingBracket(const QString &str, int start)
{
    int count = 1;
    int i = start + 1;
    while (i < str.length() && count > 0) {
        if (str.at(i) == '[') count++;
        else if (str.at(i) == ']') count--;
        i++;
    }
    return i - 1;
}

int MIParser::findMatchingBrace(const QString &str, int start)
{
    int count = 1;
    int i = start + 1;
    while (i < str.length() && count > 0) {
        if (str.at(i) == '{') count++;
        else if (str.at(i) == '}') count--;
        i++;
    }
    return i - 1;
}
