#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QObject>
#include <QSyntaxHighlighter>
#include <QTextDocument>

class SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    SyntaxHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
    QTextCharFormat numberFormat;
};

class LineNumberArea;

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    CodeEditor(QWidget *parent = nullptr);
    
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    
    // File operations
    bool openFile(const QString &filename);
    bool saveFile(const QString &filename = QString());
    bool saveFileAs(const QString &filename);
    
    QString getCurrentFile() const { return currentFile; }
    bool isModified() const { return document()->isModified(); }
    void setModified(bool modified) { document()->setModified(modified); }
    
    // Editor features
    void gotoLine(int lineNumber);
    void insertSnippet(const QString &snippet);
    void toggleLineComment();
    void indentLines();
    void unindentLines();

signals:
    void modificationChanged(bool changed);
    void cursorPositionChanged(int line, int column);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);
    void highlightCurrentLine();
    void onModificationChanged(bool changed);
    void onCursorPositionChanged();

private:
    void setupSyntaxHighlighting();
    void applyTheme();
    
    LineNumberArea *lineNumberArea;
    SyntaxHighlighter *highlighter;
    QString currentFile;
};

class LineNumberArea : public QWidget
{
    Q_OBJECT

public:
    LineNumberArea(CodeEditor *editor);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    CodeEditor *codeEditor;
};

#endif // CODEEDITOR_H
