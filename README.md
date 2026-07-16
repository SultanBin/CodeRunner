# CodeRunner

A lightweight, portable, cross-platform IDE for C and C++ — built for writing algorithms, solving problems, and running code fast. No bloat, no project wizards, no 2 GB install. Just open, write, compile, run.

Think Dev-C++ — but modern, sharp, and portable.

## Why CodeRunner?

Most C/C++ IDEs fall into two camps: heavyweight (Visual Studio, CLion) or abandoned (Dev-C++, Code::Blocks). If you just want to write a quick algorithm, test a data structure, or grind competitive programming problems, you shouldn't need a 10-minute setup.

CodeRunner is built to be the tool you unzip and start coding in.

- **Zero config** — auto-detects GCC, Clang, MinGW, or MSVC on your system
- **Portable** — runs from a USB stick, no install required
- **Fast** — native Qt UI, not Electron, not a browser
- **Cross-platform** — Windows, macOS, Linux from one codebase
- **Focused** — C and C++ only, no plugin bloat

## Features

### Core

- Syntax-highlighted code editor with line numbers and dark theme
- Create, open, save, and close files with starter templates (C, C++, headers)
- Undo / redo / cut / copy / paste / select all
- Single-file and multi-file project compilation
- Color-coded compiler output with click-to-navigate errors
- Run compiled binaries with interactive I/O
- Project file tree with create / rename / delete (disk-level operations)
- Build scripts for quick launch (`./build.sh run` / `build.bat run`)

### Compiler Integration

- Auto-detection of installed compilers (GCC, Clang, MinGW, MSVC)
- Configurable C++ standard (C++11/14/17/20)
- Optimization levels, warning flags, custom compiler arguments
- Project configuration via `.coderunner` JSON files

### Debugger (GDB)

- GDB/MI protocol integration
- Breakpoint management
- Step over / step into / step out
- Variables panel, call stack, debug console

## Screenshots

Coming soon.

## Quick Start

### Prerequisites

- **Qt** 5.15+ or 6.x
- **CMake** 3.16+
- A C++ compiler (GCC, Clang, MinGW, or MSVC)

### Build

```bash
git clone https://github.com/SultanBin/CodeRunner.git
cd CodeRunner
mkdir build && cd build
cmake ..
cmake --build .
```

### Run

```bash
# Linux / macOS
./CodeRunner

# Windows
CodeRunner.exe
```

See [INSTALLATION.md](INSTALLATION.md) for platform-specific details and troubleshooting.

## Project Structure

```text
CodeRunner/
├── src/
│   ├── main.cpp                  # Entry point
│   ├── mainwindow.cpp/h          # Application shell, menus, toolbars
│   ├── codeeditor.cpp/h          # Editor widget, syntax highlighting, line numbers
│   ├── filemanager.cpp/h         # Project file tree
│   ├── compiler.cpp/h            # Compiler detection, build execution
│   ├── debugger.cpp/h            # GDB/MI integration
│   ├── miparser.cpp/h            # GDB Machine Interface protocol parser
│   ├── errorparser.cpp/h         # Compiler error/warning extraction
│   ├── compileroutputpanel.cpp/h # Color-coded build output
│   ├── compilersettingsdialog.cpp/h # Compiler configuration UI
│   ├── projectconfig.cpp/h       # .coderunner project file I/O
│   ├── breakpointpanel.cpp/h     # Breakpoint list UI
│   ├── variablespanel.cpp/h      # Variable watch UI
│   ├── callstackpanel.cpp/h      # Call stack UI
│   ├── debugconsole.cpp/h        # GDB command console
│   └── resources.qrc             # Icons and stylesheets
├── templates/                    # Starter code templates
│   ├── default.cpp
│   └── hello.c
├── docs/                         # Development roadmap and phase details
├── CMakeLists.txt
├── build.sh                      # Linux/macOS build script
├── build.bat                     # Windows build script
├── INSTALLATION.md
└── LICENSE                       # MPL-2.0
```

## Roadmap

| Phase | Status |
| --- | --- |
| Core foundation (editor, menus, file tree) | Done |
| Compiler integration (detect, build, error parsing) | Done |
| Debug mode (GDB, breakpoints, stepping) | Done |
| MVP wiring (save, edit menu, build scripts, icons) | Done |
| File manager (new/rename/delete, templates, recent projects) | ~65% done |
| UI polish (themes, fonts, layout customization) | Planned |
| Advanced features (code completion, find/replace, git) | Planned |
| Testing and cross-platform verification | Planned |
| Distribution (installers, portable builds) | Planned |

See [docs/PHASES.md](docs/PHASES.md) for the full roadmap.

## Tech Stack

- **Language:** C++17
- **UI Framework:** Qt 5 / Qt 6 (Widgets)
- **Build System:** CMake
- **Debugger Backend:** GDB (MI2 protocol)
- **License:** MPL-2.0

## Contributing

Contributions welcome. If you've ever been frustrated by the state of lightweight C/C++ IDEs, this project is for you.

1. Fork the repo
2. Create a feature branch
3. Submit a PR

## License

[Mozilla Public License 2.0](LICENSE)
