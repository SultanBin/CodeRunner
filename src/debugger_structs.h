#ifndef DEBUGGER_STRUCTS_H
#define DEBUGGER_STRUCTS_H

#include <QString>
#include <QVector>
#include <QMap>

struct Breakpoint
{
    int id;                    // Debugger-assigned ID
    QString file;
    int line;
    int column;
    bool enabled;
    QString condition;         // Empty if none
    int hitCount;              // Times hit
    int ignoreCount;           // Hit count threshold
    bool temporary;            // Auto-delete after hit
    enum Type { Line, Function, Watchpoint } type;
    
    bool isValid() const { return id >= 0 && !file.isEmpty() && line > 0; }
};

struct Variable
{
    QString name;
    QString value;
    QString type;
    QString address;           // Memory address
    bool isExpandable;         // Has members/elements
    int numChildren;           // Number of children
    QVector<Variable> members; // For structs/arrays
    
    QString toString() const {
        return QString("%1 %2 = %3").arg(type, name, value);
    }
};

struct StackFrame
{
    int level;                 // 0 = innermost
    QString address;           // Instruction address
    QString function;          // Function name
    QString file;              // Source file
    int line;                  // Line number
    int column;                // Column number
    QMap<QString, QString> args; // Function arguments
    
    QString toString() const {
        return QString("#%1 0x%2 in %3() at %4:%5").arg(
            QString::number(level), address, function, file, QString::number(line)
        );
    }
};

struct ThreadInfo
{
    int id;                    // Thread ID
    QString state;             // "running", "stopped", etc.
    QString target_id;         // OS thread ID
    bool is_running;
};

struct DebugEvent
{
    enum Type {
        Stopped,               // Program stopped (breakpoint, step, etc.)
        Running,               // Program executing
        ThreadCreated,         // New thread created
        ThreadExited,          // Thread terminated
        BreakpointModified,    // Breakpoint changed
        MemoryModified,        // Memory changed
        SignalReceived         // Signal caught
    } type;
    
    QString reason;            // "breakpoint-hit", "end-stepping-range", etc.
    int threadId;
    QString signalName;
    QString signalMeaning;
    QString stoppedThreads;    // List of stopped threads
};

#endif // DEBUGGER_STRUCTS_H
