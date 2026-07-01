#include "debugconsole.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>

DebugConsole::DebugConsole(QWidget *parent)
    : QWidget(parent), historyIndex(-1)
{
    setupUI();
    applyTheme();
}

void DebugConsole::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // Output area
    outputArea = new QPlainTextEdit();
    outputArea->setReadOnly(true);
    outputArea->setPlaceholderText(tr("Debug console output..."));
    outputArea->setFont(QFont("Courier New", 10));
    layout->addWidget(outputArea);
    
    // Command input
    QHBoxLayout *inputLayout = new QHBoxLayout();
    commandInput = new QLineEdit();
    commandInput->setPlaceholderText(tr("(gdb) "));
    commandInput->setFont(QFont("Courier New", 10));
    
    sendBtn = new QPushButton(tr("Send"));
    clearBtn = new QPushButton(tr("Clear"));
    
    inputLayout->addWidget(commandInput);
    inputLayout->addWidget(sendBtn);
    inputLayout->addWidget(clearBtn);
    layout->addLayout(inputLayout);
    
    // Connect signals
    connect(commandInput, &QLineEdit::returnPressed, this, &DebugConsole::onCommandEntered);
    connect(sendBtn, &QPushButton::clicked, this, &DebugConsole::onCommandEntered);
    connect(clearBtn, &QPushButton::clicked, this, &DebugConsole::onClearClicked);
}

void DebugConsole::applyTheme()
{
    setStyleSheet(
        "QPlainTextEdit { "
        "background-color: #1e1e1e; "
        "color: #d4d4d4; "
        "border: 1px solid #3c3c3c; "
        "selection-background-color: #0078d4; "
        "} "
        "QLineEdit { "
        "background-color: #3c3c3c; "
        "color: #d4d4d4; "
        "border: 1px solid #555555; "
        "padding: 5px; "
        "font-weight: bold; "
        "}"
    );
}

void DebugConsole::appendOutput(const QString &text)
{
    outputArea->appendPlainText(text);
}

void DebugConsole::appendCommand(const QString &cmd)
{
    outputArea->appendPlainText(QString("(gdb) %1").arg(cmd));
}

void DebugConsole::appendResponse(const QString &response)
{
    outputArea->appendPlainText(response);
}

void DebugConsole::appendError(const QString &error)
{
    QString errorLine = QString("<span style='color: #ff6b6b;'>%1</span>").arg(error);
    outputArea->appendHtml(errorLine);
}

void DebugConsole::clear()
{
    outputArea->clear();
}

void DebugConsole::onCommandEntered()
{
    QString cmd = commandInput->text().trimmed();
    if (!cmd.isEmpty()) {
        appendCommand(cmd);
        addCommandToHistory(cmd);
        emit commandExecuted(cmd);
        commandInput->clear();
        historyIndex = -1;
    }
}

void DebugConsole::onPreviousCommand()
{
    if (commandHistory.isEmpty()) return;
    
    if (historyIndex < 0) {
        historyIndex = commandHistory.count() - 1;
    } else if (historyIndex > 0) {
        historyIndex--;
    }
    
    if (historyIndex >= 0 && historyIndex < commandHistory.count()) {
        commandInput->setText(commandHistory[historyIndex]);
    }
}

void DebugConsole::onNextCommand()
{
    if (commandHistory.isEmpty()) return;
    
    if (historyIndex < commandHistory.count() - 1) {
        historyIndex++;
        commandInput->setText(commandHistory[historyIndex]);
    } else {
        historyIndex = -1;
        commandInput->clear();
    }
}

void DebugConsole::onClearClicked()
{
    clear();
}

void DebugConsole::addCommandToHistory(const QString &cmd)
{
    if (commandHistory.isEmpty() || commandHistory.last() != cmd) {
        commandHistory.append(cmd);
    }
}
