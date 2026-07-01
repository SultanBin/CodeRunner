#ifndef DEBUGCONSOLE_H
#define DEBUGCONSOLE_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QStringList>

class DebugConsole : public QWidget
{
    Q_OBJECT

public:
    explicit DebugConsole(QWidget *parent = nullptr);
    
    // Console output
    void appendOutput(const QString &text);
    void appendCommand(const QString &cmd);
    void appendResponse(const QString &response);
    void appendError(const QString &error);
    void clear();
    
    // Command history
    QStringList getCommandHistory() const { return commandHistory; }
    
signals:
    void commandExecuted(const QString &command);

private slots:
    void onCommandEntered();
    void onPreviousCommand();
    void onNextCommand();
    void onClearClicked();

private:
    void setupUI();
    void applyTheme();
    void addCommandToHistory(const QString &cmd);
    
    QPlainTextEdit *outputArea;
    QLineEdit *commandInput;
    QPushButton *sendBtn;
    QPushButton *clearBtn;
    
    QStringList commandHistory;
    int historyIndex;
};

#endif // DEBUGCONSOLE_H
