# Phase 3: Debug Mode Integration

## Overview
Phase 3 implements full debugging capabilities using GDB/LLDB. Users can now:
- Start debugging compiled programs
- Set and manage breakpoints
- Step through code (step over, step into, step out)
- Inspect variables and memory
- View call stack
- Watch expressions
- Debug console for direct debugger commands

## Implementation Details

### 1. Enhanced Debugger Class

#### GDB/LLDB Integration:
- **Dual support** - Automatically selects GDB or LLDB based on platform
- **MI interface** - Uses GDB Machine Interface (MI2) for structured output
- **Process communication** - Bidirectional stdin/stdout/stderr
- **Asynchronous events** - Parse debugger events (breakpoint hit, execution stop, etc.)

#### Key Methods:
```cpp
// Execution control
void startDebug(const QString &executable);
void stopDebug();
void stepOver();
void stepInto();
void stepOut();
void continueExecution();
void runToLine(const QString &file, int line);
void pause();

// Breakpoints
void setBreakpoint(const QString &file, int line);
void deleteBreakpoint(int id);
void toggleBreakpoint(const QString &file, int line);
QVector<Breakpoint> getBreakpoints() const;

// Variables & inspection
void inspectVariable(const QString &name);
QString getVariableValue(const QString &name);
QVector<Variable> getLocalVariables();
QVector<Variable> getGlobalVariables();

// Call stack
QVector<StackFrame> getCallStack();
void selectStackFrame(int index);

// Memory
QByteArray readMemory(uint64_t address, int size);
void writeMemory(uint64_t address, const QByteArray &data);
```

### 2. Debug Widgets

#### A. BreakpointPanel
- List all active breakpoints
- Enable/disable breakpoints
- Delete breakpoints
- Conditional breakpoints (when expression is true)
- Hit count tracking

#### B. VariablesPanel
- **Local variables** - Current scope variables
- **Global variables** - Program globals
- **Watches** - User-defined variable watches
- **Expandable tree** - Navigate nested structures
- **Value editing** - Change variable values at runtime

#### C. CallStackPanel
- Display call stack frames
- Click to select frame
- Show file:line for each frame
- Display function names

#### D. DebugConsole
- Direct GDB/LLDB command execution
- Command history
- Syntax highlighting for GDB commands
- Auto-completion for common commands

### 3. Breakpoint Management

#### Breakpoint Types:
```cpp
struct Breakpoint {
    int id;                    // Debugger-assigned ID
    QString file;
    int line;
    bool enabled;
    QString condition;         // Optional condition
    int hitCount;
    int maxHits;              // 0 = unlimited
    enum Type { Line, Function, Watchpoint } type;
};
```

#### Features:
- **Line breakpoints** - Break at specific line
- **Function breakpoints** - Break at function entry
- **Conditional breakpoints** - `i > 10 && x == 0`
- **Hit count** - Break after N hits
- **Temporary breakpoints** - Auto-delete after hit
- **Disabled state** - Skip without deleting

### 4. Variable Inspection

#### Variable Structure:
```cpp
struct Variable {
    QString name;
    QString value;
    QString type;
    QString address;          // Memory address
    bool isExpandable;        // Has members/elements
    QVector<Variable> members; // For structs/arrays
};
```

#### Features:
- **Pretty printing** - Format complex types nicely
- **Expandable tree** - Navigate structures/arrays
- **Value modification** - Edit variable values
- **Memory address** - Show variable address
- **Type information** - Display full type

### 5. Debug UI Layout

```
┌─────────────────────────────────────────────────┐
│  Code Editor (with line numbers & breakpoints)  │
├──────────────────┬──────────────────────────────┤
│  Call Stack      │  Variables & Watches Panel   │
├──────────────────┤  • Local Variables           │
│  Breakpoints     │  • Global Variables          │
│  • id  file:line │  • Watches                   │
│  [x] main.cpp:15 │                              │
│  [ ] helper.cpp:42                              │
├──────────────────┴──────────────────────────────┤
│  Debug Console (GDB/LLDB commands)              │
│  (gdb) print x                                   │
│  $1 = 42                                         │
└──────────────────────────────────────────────────┘
```

### 6. Debug Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| F5 | Start/Resume debug |
| F10 | Step Over |
| F11 | Step Into |
| Shift+F11 | Step Out |
| Ctrl+B | Toggle Breakpoint |
| Shift+F5 | Stop Debug |
| F6 | Continue |
| Ctrl+G | Go to Line (and run to) |

### 7. MI (Machine Interface) Protocol

#### GDB MI Commands:
```
# Start in MI mode
gdb --interpreter=mi2 <program>

# Breakpoint operations
-break-insert file.cpp:10
-break-delete 1
-break-enable 1
-break-disable 1

# Execution
-exec-run
-exec-next
-exec-step
-exec-finish
-exec-continue

# Stack & variables
-stack-list-frames
-stack-list-arguments
-stack-list-locals
-var-create
-var-list-children
-var-evaluate-expression
```

### 8. MI Output Parsing

#### Parser Implementation:
```cpp
class MIParser {
    struct ParsedResult {
        QString resultClass;    // "done", "running", "connected", "error", "exit"
        QMap<QString, QVariant> results;
        QVector<MIEvent> events;
    };
    
    ParsedResult parse(const QString &output);
};
```

#### Event Types:
- `=thread-group-added`
- `=thread-created`
- `=thread-exited`
- `=breakpoint-created`
- `=breakpoint-modified`
- `=breakpoint-deleted`
- `*stopped` - Execution stopped (breakpoint, step, etc.)
- `*running` - Program executing

---

## Files Modified/Added

### Enhanced Files:
- `src/debugger.cpp` - Full GDB/LLDB MI implementation
- `src/mainwindow.cpp` - Debug menu/toolbar integration
- `src/mainwindow.h` - Debug widget signals

### New Files:
- `src/debugger.h` - Enhanced debugger interface
- `src/miparser.h/cpp` - MI protocol parser
- `src/breakpointpanel.h/cpp` - Breakpoint management UI
- `src/variablespanel.h/cpp` - Variables inspector
- `src/callstackpanel.h/cpp` - Call stack viewer
- `src/debugconsole.h/cpp` - GDB console
- `src/debugsettings.h/cpp` - Debug configuration
- `src/debugger_structs.h` - Data structures

---

## Testing Checklist

- [ ] Start debugging executable
- [ ] Set breakpoint at line
- [ ] Hit breakpoint and pause execution
- [ ] Step Over (F10)
- [ ] Step Into (F11)
- [ ] Step Out (Shift+F11)
- [ ] Continue execution
- [ ] Inspect local variables
- [ ] Inspect global variables
- [ ] Add watch expression
- [ ] Modify variable value
- [ ] View call stack
- [ ] Click stack frame to navigate
- [ ] Conditional breakpoint
- [ ] Temporary breakpoint
- [ ] Debug console command execution
- [ ] Stop debugging

---

## Platform Support

### Linux/macOS:
- GDB (GNU Debugger) - Default
- LLDB (LLVM Debugger) - Alternative

### Windows:
- GDB (from MinGW toolchain)
- CDB (Visual Studio Debugger) - For MSVC
- LLDB (with Clang toolchain)

---

## Architecture Diagram

```
┌─────────────────────┐
│  CodeRunner (Qt)    │
├─────────────────────┤
│  DebuggerControl    │ <- Main control logic
├──────────┬──────────┤
│  MIParser│GDBProcess│ <- GDB/LLDB communication
└──────────┴──────────┘
        ↓
┌─────────────────────┐
│  GDB/LLDB Process   │
└─────────────────────┘
```

---

## Next Phase

**Phase 4: File Manager Enhancement** will add:
- Create/delete files and folders
- Rename operations
- Project templates
- Drag & drop file organization
- .gitignore support
