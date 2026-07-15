#include "compileroutputpanel.h"
#include "errorparser.h"
#include <QRegularExpression>
#include <QFileInfo>

// Incremental feeder: buffer partial chunks until newline, avoid clearing panel
static QString g_compiler_output_buffer;

void feedOutputToPanel(CompilerOutputPanel *panel, const QString &chunk)
{
    if (!panel) return;

    ErrorParser parser;

    // Append incoming chunk to buffer
    g_compiler_output_buffer += chunk;
    // Normalize CRLF
    g_compiler_output_buffer.replace("\r\n", "\n");

    int lastNewline = g_compiler_output_buffer.lastIndexOf('\n');
    if (lastNewline == -1) {
        // No complete lines yet
        return;
    }

    QString toParse = g_compiler_output_buffer.left(lastNewline + 1);
    g_compiler_output_buffer = g_compiler_output_buffer.mid(lastNewline + 1);

    QStringList lines = toParse.split('\n', Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        if (line.trimmed().isEmpty()) continue;
        QVector<ParsedError> parsed = parser.parseOutput(line);
        if (!parsed.isEmpty()) {
            for (const ParsedError &pe : parsed) {
                CompileError ce;
                QFileInfo fi(pe.filename);
                ce.file = fi.filePath();
                ce.line = pe.lineNumber;
                ce.column = pe.columnNumber;
                ce.level = pe.severity;
                ce.message = pe.message;
                ce.raw = pe.fullLine;
                panel->addErrorLine(ce);
            }
        } else {
            panel->addOutput(line);
        }
    }
}
