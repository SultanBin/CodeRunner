#ifndef COMPILEROUTPUTPANEL_H
#define COMPILEROUTPUTPANEL_H

#include <QPlainTextEdit>
#include <QSyntaxHighlighter>
#include <QTextDocument>
#include <QVector>

struct CompileError
{
    QString file;
    int line;
    int column;
    QString level;      // "error", "warning", "note"
    QString message;
    QString raw;
};

class CompilerOutputHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    CompilerOutputHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    QTextCharFormat errorFormat;
    QTextCharFormat warningFormat;
    QTextCharFormat successFormat;
    QTextCharFormat noteFormat;
};

class CompilerOutputPanel : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit CompilerOutputPanel(QWidget *parent = nullptr);
    
    // Add output
    void addOutput(const QString &text);
    void addErrorLine(const CompileError &error);
    void clear();
    
    // Error navigation
    void nextError();
    void previousError();
    
    // Error list
    QVector<CompileError> getErrors() const { return errors; }
    int getErrorCount() const { return errors.count(); }
    
signals:
    void errorClicked(const QString &file, int line, int column);
    void errorCountChanged(int count, int warnings);

private slots:
    void onTextChanged();
    void onCursorPositionChanged();

private:
    void parseAndHighlightErrors();
    CompileError parseErrorLine(const QString &line);
    void navigateToError(int index);
    void applyTheme();
    
    QVector<CompileError> errors;
    int currentErrorIndex;
    CompilerOutputHighlighter *highlighter;
};

#endif // COMPILEROUTPUTPANEL_H
