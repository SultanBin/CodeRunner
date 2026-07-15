#include "compileroutputpanel.h"
#include "errorparser.h"
#include <QRegularExpression>

// Connect compiler output to error parser

void integrateCompilerOutputWithParser(CompilerOutputPanel *panel, ErrorParser *parser)
{
    // Whenever the panel is updated, reparse and emit clicks when lines selected.
    // This helper is intentionally small — we'll wire the signal connections in MainWindow.
    Q_UNUSED(panel);
    Q_UNUSED(parser);
}
