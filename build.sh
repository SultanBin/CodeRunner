#!/bin/bash
set -e

BUILD_DIR="build"
BINARY="$BUILD_DIR/bin/CodeRunner"

case "${1:-build}" in
    build)
        mkdir -p "$BUILD_DIR"
        cd "$BUILD_DIR"
        cmake ..
        cmake --build .
        echo "Build complete: $BINARY"
        ;;
    run)
        if [ ! -f "$BINARY" ]; then
            echo "Binary not found, building first..."
            "$0" build
        fi
        echo "Running CodeRunner..."
        "$BINARY"
        ;;
    clean)
        rm -rf "$BUILD_DIR"
        echo "Build directory cleaned."
        ;;
    rebuild)
        "$0" clean
        "$0" build
        ;;
    *)
        echo "Usage: ./build.sh [build|run|clean|rebuild]"
        echo "  build   - Configure and compile (default)"
        echo "  run     - Build if needed, then launch"
        echo "  clean   - Remove build directory"
        echo "  rebuild - Clean + build"
        exit 1
        ;;
esac
