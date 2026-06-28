#include "compileroutputpanel.h"
#include <QPainter>
#include <QTextBlock>
#include <QDebug>
#include <QRegularExpression>
#include <QScrollBar>
#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QMenu>

// Compiler Output Highlighter Implementation
CompilerOutputHighlighter::CompilerOutputHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    errorFormat.setForeground(QColor(255, 255, 255));
    errorFormat.setBackground(QColor(200, 0, 0));     // Red background
    errorFormat.setFontWeight(QFont::Bold);
    
    warningFormat.setForeground(QColor(255, 200, 0)); // Yellow
    warningFormat.setFontWeight(QFont::Bold);
    
    successFormat.setForeground(QColor(0, 200, 0));   // Green
    successFormat.setFontWeight(QFont::Bold);
    
    noteFormat.setForeground(QColor(100, 150, 200));  // Light blue
}

void CompilerOutputHighlighter::highlightBlock(const QString &text)
{
    if (text.contains("error", Qt::CaseInsensitive)) {
        setFormat(0, text.length(), errorFormat);
    } else if (text.contains("warning", Qt::CaseInsensitive)) {
        setFormat(0, text.length(), warningFormat);
    } else if (text.contains("success", Qt::CaseInsensitive) || 
               text.contains("successfully", Qt::CaseInsensitive)) {
        setFormat(0, text.length(), successFormat);
    } else if (text.contains("note", Qt::CaseInsensitive)) {
        setFormat(0, text.length(), noteFormat);
    }
}

// Compiler Output Panel Implementation
CompilerOutputPanel::CompilerOutputPanel(QWidget *parent)
    : QPlainTextEdit(parent), currentErrorIndex(-1)
{
    setReadOnly(true);
    setPlaceholderText(tr("Build output will appear here..."));
    
    highlighter = new CompilerOutputHighlighter(document());
    
    applyTheme();
    
    // Setup fonts
    QFont font("Courier New", 10);
    setFont(font);
    setTabStopDistance(40);
    
    // Connect signals
    connect(this, &QPlainTextEdit::textChanged, this, &CompilerOutputPanel::onTextChanged);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &CompilerOutputPanel::onCursorPositionChanged);
}

void CompilerOutputPanel::addOutput(const QString &text)
{
    appendPlainText(text);
    // Scroll to bottom
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

void CompilerOutputPanel::addErrorLine(const CompileError &error)
{
    QString formattedLine;
    
    if (error.level == "error") {
        formattedLine = QString("[ERROR] %1:%2:%3: %4").arg(error.file, 
                                                             QString::number(error.line),
                                                             QString::number(error.column),
                                                             error.message);
    } else if (error.level == "warning") {
        formattedLine = QString("[WARNING] %1:%2:%3: %4").arg(error.file,
                                                               QString::number(error.line),
                                                               QString::number(error.column),
                                                               error.message);
    } else {
        formattedLine = error.raw;
    }
    
    appendPlainText(formattedLine);
    errors.append(error);
    
    emit errorCountChanged(errors.count(), 
                          errors.count(1) + errors.count(2));
}

void CompilerOutputPanel::clear()
{
    QPlainTextEdit::clear();
    errors.clear();
    currentErrorIndex = -1;
    emit errorCountChanged(0, 0);
}

void CompilerOutputPanel::nextError()
{
    if (errors.isEmpty()) return;
    
    currentErrorIndex++;
    if (currentErrorIndex >= errors.count()) {
        currentErrorIndex = 0;
    }
    
    navigateToError(currentErrorIndex);
}

void CompilerOutputPanel::previousError()
{
    if (errors.isEmpty()) return;
    
    currentErrorIndex--;
    if (currentErrorIndex < 0) {
        currentErrorIndex = errors.count() - 1;
    }
    
    navigateToError(currentErrorIndex);
}

void CompilerOutputPanel::onTextChanged()
{
    parseAndHighlightErrors();
}

void CompilerOutputPanel::onCursorPositionChanged()
{
    // Highlight current line
}

void CompilerOutputPanel::parseAndHighlightErrors()
{
    errors.clear();
    
    QStringList lines = toPlainText().split('\n');
    for (const QString &line : lines) {
        CompileError error = parseErrorLine(line);
        if (!error.file.isEmpty()) {
            errors.append(error);
        }
    }
}

CompileError CompilerOutputPanel::parseErrorLine(const QString &line)
{
    CompileError error;
    error.raw = line;
    
    // GCC/Clang format: file:line:col: error/warning: message
    QRegularExpression gccPattern("^([^:]+):(\\d+):(\\d+):\\s*(error|warning|note):\\s*(.*)$");
    QRegularExpressionMatch match = gccPattern.match(line);
    
    if (match.hasMatch()) {
        error.file = match.captured(1);
        error.line = match.captured(2).toInt();
        error.column = match.captured(3).toInt();
        error.level = match.captured(4);
        error.message = match.captured(5);
        return error;
    }
    
    // MSVC format: file(line): error C1234: message
    QRegularExpression msvcPattern("^([^(]+)\\((\\d+)\\):\\s*(error|warning)\\s*([A-Z]\\d+):\\s*(.*)$");
    match = msvcPattern.match(line);
    
    if (match.hasMatch()) {
        error.file = match.captured(1);
        error.line = match.captured(2).toInt();
        error.level = match.captured(3);
        error.message = match.captured(5);
        error.column = 1;
        return error;
    }
    
    // If no match, return empty error
    error.file.clear();
    return error;
}

void CompilerOutputPanel::navigateToError(int index)
{
    if (index < 0 || index >= errors.count()) return;
    
    const CompileError &error = errors[index];
    emit errorClicked(error.file, error.line, error.column);
}

void CompilerOutputPanel::applyTheme()
{
    setStyleSheet(
        "QPlainTextEdit { "
        "background-color: #1e1e1e; "
        "color: #d4d4d4; "
        "border: 1px solid #3c3c3c; "
        "selection-background-color: #0078d4; "
        "}"
    );
}
