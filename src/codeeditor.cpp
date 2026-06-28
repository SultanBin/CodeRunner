#include "codeeditor.h"
#include <QPainter>
#include <QTextBlock>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QTextBlockUserData>
#include <QApplication>
#include <QMimeData>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDesktopServices>
#include <QUrl>

// Syntax Highlighter Implementation
SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    // Keyword format
    keywordFormat.setForeground(QColor(86, 156, 214)); // Blue
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\b(if|else|switch|case|default|for|while|do|break|continue)\\b"
                    << "\\b(return|void|int|float|double|char|bool|long|short|unsigned|signed)\\b"
                    << "\\b(struct|class|union|enum|typedef|const|static|volatile|extern)\\b"
                    << "\\b(sizeof|typedef|namespace|using|inline|virtual|override|final)\\b"
                    << "\\b(template|typename|public|private|protected|this|nullptr|auto)\\b";

    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Class/type format
    classFormat.setForeground(QColor(78, 201, 176)); // Teal
    classFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("\\b[A-Z]\\w*\\b");
    rule.format = classFormat;
    highlightingRules.append(rule);

    // Number format
    numberFormat.setForeground(QColor(181, 206, 168)); // Green
    rule.pattern = QRegularExpression("\\b\\d+(\\.\\d+)?\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    // Function format
    functionFormat.setForeground(QColor(220, 220, 170)); // Yellow
    rule.pattern = QRegularExpression("\\b[a-z_]\\w*(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    // Quotation format
    quotationFormat.setForeground(QColor(206, 145, 120)); // Orange
    rule.pattern = QRegularExpression("\".*?\"|\'.*?\'");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // Single line comment
    singleLineCommentFormat.setForeground(QColor(87, 166, 74)); // Green
    singleLineCommentFormat.setFontItalic(true);
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // Multi-line comment
    multiLineCommentFormat.setForeground(QColor(87, 166, 74)); // Green
    multiLineCommentFormat.setFontItalic(true);
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    // Apply regular expression rules
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    // Highlight multi-line comments
    QRegularExpression startExpression("/\\*");
    QRegularExpression endExpression("\\*/");

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(startExpression);

    while (startIndex >= 0) {
        QRegularExpressionMatch endMatch = endExpression.match(text, startIndex);
        int endIndex = endMatch.capturedStart();
        int commentLength;
        if (endIndex == -1) {
            setBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + endMatch.capturedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(startExpression, startIndex + commentLength);
    }
}

// Line Number Area Implementation
LineNumberArea::LineNumberArea(CodeEditor *editor)
    : QWidget(editor), codeEditor(editor)
{
}

QSize LineNumberArea::sizeHint() const
{
    return QSize(codeEditor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    codeEditor->lineNumberAreaPaintEvent(event);
}

// Code Editor Implementation
CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);
    highlighter = new SyntaxHighlighter(document());

    setFont(QFont("Courier New", 11));
    setTabStopDistance(40);
    setAcceptDrops(true);

    setupSyntaxHighlighting();
    applyTheme();

    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);
    connect(document(), &QTextDocument::modificationChanged, this, &CodeEditor::onModificationChanged);
    connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::onCursorPositionChanged);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

void CodeEditor::setupSyntaxHighlighting()
{
    // Syntax highlighting is set up in SyntaxHighlighter class
}

void CodeEditor::applyTheme()
{
    // Dark theme
    setStyleSheet(
        "QPlainTextEdit { "
        "background-color: #1e1e1e; "
        "color: #d4d4d4; "
        "border: none; "
        "selection-background-color: #0078d4; "
        "}"
    );
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() == Qt::ControlModifier) {
        int delta = event->angleDelta().y();
        if (delta > 0) {
            zoomIn();
        } else {
            zoomOut();
        }
        event->accept();
    } else {
        QPlainTextEdit::wheelEvent(event);
    }
}

void CodeEditor::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Tab) {
        insertPlainText("    ");
        event->accept();
    } else {
        QPlainTextEdit::keyPressEvent(event);
    }
}

void CodeEditor::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    } else {
        QPlainTextEdit::dragEnterEvent(event);
    }
}

void CodeEditor::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        foreach (const QUrl &url, mimeData->urls()) {
            QString filename = url.toLocalFile();
            if (openFile(filename)) {
                event->acceptProposedAction();
                return;
            }
        }
    }
    QPlainTextEdit::dropEvent(event);
}

void CodeEditor::updateLineNumberAreaWidth(int newBlockCount)
{
    Q_UNUSED(newBlockCount);
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), QColor(30, 30, 30));
    painter.setPen(QColor(100, 100, 100));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.drawText(0, top, lineNumberArea->width() - 4, fontMetrics().height(),
                           Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor(60, 60, 60);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();

        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

bool CodeEditor::openFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "Cannot read file" << filename;
        return false;
    }

    QTextStream in(&file);
    setPlainText(in.readAll());
    file.close();

    currentFile = filename;
    document()->setModified(false);
    return true;
}

bool CodeEditor::saveFile(const QString &filename)
{
    QString name = filename.isEmpty() ? currentFile : filename;
    if (name.isEmpty()) return false;

    QFile file(name);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        qWarning() << "Cannot write file" << name;
        return false;
    }

    QTextStream out(&file);
    out << toPlainText();
    file.close();

    currentFile = name;
    document()->setModified(false);
    return true;
}

bool CodeEditor::saveFileAs(const QString &filename)
{
    return saveFile(filename);
}

void CodeEditor::gotoLine(int lineNumber)
{
    if (lineNumber < 1) return;

    QTextBlock block = document()->findBlockByNumber(lineNumber - 1);
    if (block.isValid()) {
        QTextCursor cursor(block);
        setTextCursor(cursor);
    }
}

void CodeEditor::insertSnippet(const QString &snippet)
{
    insertPlainText(snippet);
}

void CodeEditor::toggleLineComment()
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfLine);

    QString text = cursor.block().text();
    if (text.trimmed().startsWith("//")) {
        text.replace(QRegularExpression("^(\\s*)//"), "\\1");
    } else {
        text.replace(QRegularExpression("^(\\s*)"), "\\1//");
    }

    cursor.select(QTextCursor::LineUnderCursor);
    cursor.insertText(text);
}

void CodeEditor::indentLines()
{
    // Indent selected lines
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();

    while (cursor.movePosition(QTextCursor::StartOfLine)) {
        cursor.insertText("    ");
        if (!cursor.movePosition(QTextCursor::Down))
            break;
    }

    cursor.endEditBlock();
}

void CodeEditor::unindentLines()
{
    // Unindent selected lines
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();

    while (cursor.movePosition(QTextCursor::StartOfLine)) {
        QString text = cursor.block().text();
        if (text.startsWith("    ")) {
            cursor.deleteChar();
            cursor.deleteChar();
            cursor.deleteChar();
            cursor.deleteChar();
        }
        if (!cursor.movePosition(QTextCursor::Down))
            break;
    }

    cursor.endEditBlock();
}

void CodeEditor::onModificationChanged(bool changed)
{
    emit modificationChanged(changed);
}

void CodeEditor::onCursorPositionChanged()
{
    QTextCursor cursor = textCursor();
    int line = cursor.blockNumber() + 1;
    int column = cursor.positionInBlock() + 1;
    emit cursorPositionChanged(line, column);
}
