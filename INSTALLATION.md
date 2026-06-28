# CodeRunner - Installation & Build Guide

## Prerequisites

### Windows
- Qt 5.15+ or Qt 6.x
- CMake 3.16+
- MinGW (bundled with Qt) or Visual Studio Build Tools
- GCC or Clang compiler

### macOS
- Qt 5.15+ or Qt 6.x
- CMake 3.16+
- Xcode Command Line Tools
- GCC or Clang (comes with Xcode)

### Linux (Ubuntu/Debian)
```bash
sudo apt-get install qt5-qmake qt5-default libqt5core5a libqt5gui5 libqt5widgets5
sudo apt-get install cmake build-essential
sudo apt-get install gcc g++ gdb
```

## Build Instructions

### 1. Clone the Repository
```bash
git clone https://github.com/SultanBin/CodeRunner.git
cd CodeRunner
```

### 2. Create Build Directory
```bash
mkdir build
cd build
```

### 3. Configure with CMake
```bash
cmake ..
```

### 4. Build
```bash
# Unix-like systems
make

# or on all platforms
cmake --build .
```

### 5. Run
```bash
# Unix-like
./CodeRunner

# Windows
CodeRunner.exe
```

## Platform-Specific Notes

### Windows with Visual Studio
```bash
cmake -G "Visual Studio 16 2019" ..
cmake --build . --config Release
```

### macOS
```bash
brew install qt cmake
cmake -DCMAKE_PREFIX_PATH=$(brew --prefix qt) ..
make
```

## Troubleshooting

### CMake can't find Qt
```bash
# Set Qt path manually
cmake -DCMAKE_PREFIX_PATH=/path/to/qt ..
```

### Compiler not found
Make sure GCC, Clang, or MSVC is installed and in PATH:
```bash
g++ --version
clang++ --version
```

### Missing dependencies
- Ubuntu: `sudo apt-get install qt5-default`
- macOS: `brew install qt5`
- Windows: Download Qt from https://www.qt.io/download

## Next Steps

See [PHASES.md](docs/PHASES.md) for the development roadmap.
