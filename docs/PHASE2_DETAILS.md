# Phase 2: Compiler Integration

## Overview
Phase 2 focuses on making the compiler fully functional. Users can now:
- Compile C/C++ files with auto-detected compilers
- View colored build output with error/warning highlighting
- Navigate to errors directly in the editor
- Configure compiler settings and flags

## Implementation Details

### 1. Enhanced Compiler Class

#### Features:
- **Auto-detection** of GCC, Clang, MinGW, MSVC
- **Real-time output** streaming
- **Error parsing** (extract file, line, message)
- **Warning/Error highlighting** with color coding
- **Custom flags** and optimization levels
- **Include/Library paths** management

#### Key Methods:
```cpp
// Compilation
void compileFile(const QString &sourceFile, const QString &outputFile);
void compileAndRun(const QString &sourceFile);
void stop();

// Configuration
void setOptimizationLevel(const QString &level);  // -O0, -O1, -O2, -O3
void setStandard(const QString &standard);         // c++11, c++17, c++20
void addWarnings(bool enable);
void addDebugInfo(bool enable);

// Detection
QStringList getAvailableCompilers();
QStringList getAvailableStandards();
```

### 2. Output Console with Colors

#### Features:
- **Error lines**: Red background
- **Warnings**: Yellow text
- **Success**: Green text
- **Info**: Default text
- **Scrollable** with search capability
- **Clickable** error links

#### Implementation:
```cpp
class CompilerOutputPanel : public QPlainTextEdit {
    void highlightLine(int lineNum, ErrorLevel level);
    void onErrorClicked(const QString &file, int line);
};
```

### 3. Compiler Settings Dialog

#### Options:
- **Compiler Selection** (GCC, Clang, MinGW, MSVC)
- **C++ Standard** (C++11, C++14, C++17, C++20)
- **Optimization Level** (-O0, -O1, -O2, -O3, -Os)
- **Warning Level** (None, All, Extra)
- **Debug Info** (On/Off)
- **Include Paths** (add/remove)
- **Library Paths** (add/remove)
- **Custom Flags**

### 4. Project Configuration File

#### `.coderunner` File Format:
```json
{
  "project": "MyProject",
  "compiler": "gcc",
  "standard": "c++17",
  "optimization": "-O2",
  "warnings": true,
  "debug": true,
  "includePaths": [
    "/usr/include",
    "./include"
  ],
  "libraryPaths": [
    "/usr/lib",
    "./lib"
  ],
  "libraries": ["m", "pthread"],
  "customFlags": "-Wall -Wextra",
  "sourceDir": "src",
  "buildDir": "build",
  "binDir": "bin"
}
```

### 5. Error Parsing

#### Supported Formats:
- **GCC/Clang**: `file:line:col: error: message`
- **MSVC**: `file(line): error C1234: message`
- **Generic**: Regex patterns for custom compiler formats

#### Error Extraction:
```cpp
struct CompileError {
    QString file;
    int line;
    int column;
    QString level;     // "error", "warning", "note"
    QString message;
    QString raw;       // Full error line
};

QVector<CompileError> parseCompilerOutput(const QString &output);
```

### 6. Compile & Run Integration

#### Workflow:
1. User clicks "Compile & Run"
2. Save current file (if modified)
3. Detect output executable name
4. Compile with stored settings
5. On success, automatically run executable
6. Capture and display program output

### 7. Build Shortcuts

#### Keyboard Shortcuts:
- **Ctrl+F9**: Compile only
- **Ctrl+F10**: Compile & Run
- **Ctrl+Shift+F5**: Stop compilation
- **F7**: Jump to next error
- **Shift+F7**: Jump to previous error

---

## Files Modified/Added

### Modified:
- `src/compiler.cpp` - Enhanced with error parsing
- `src/mainwindow.cpp` - Connect compile buttons
- `src/mainwindow.h` - Add compiler config dialog

### New Files:
- `src/compileroutputpanel.h/cpp` - Colored output console
- `src/compilersettingsdialog.h/cpp` - Settings UI
- `src/projectconfig.h/cpp` - Project configuration
- `src/errorparser.h/cpp` - Parse compiler output

---

## Testing Checklist

- [ ] Detect installed compilers on system
- [ ] Compile simple C file
- [ ] Compile simple C++ file
- [ ] Display compilation errors with colors
- [ ] Clickable error navigation
- [ ] Display compilation warnings
- [ ] Save/load compiler settings
- [ ] Compile & Run executes program
- [ ] Program output displayed in console
- [ ] Stop compilation button works
- [ ] Error highlighting in editor
- [ ] Custom compiler flags applied

---

## Next Phase

**Phase 3: Debug Mode** will add:
- GDB/LLDB integration
- Breakpoints
- Step through code
- Variable inspection
- Call stack
