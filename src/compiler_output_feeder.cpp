#include "compileroutputpanel.h"
#include "errorparser.h"
#include <QRegularExpression>
#include <QFileInfo>

// Helper: feed compiler output through ErrorParser, populate CompilerOutputPanel with structured errors
void feedOutputToPanel(CompilerOutputPanel *panel, const QString &output)
{
    if (!panel) return;

    ErrorParser parser;
    panel->clear();

    // Split output into lines and parse
    QStringList lines = output.split('\n');
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
