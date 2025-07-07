#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Directories
SRC_DIR="bumi"
TEST_DIR="tests"
INCLUDE_DIR="bumi"
BIN_DIR="bin"
MAIN_BINARY="bumi"
TEST_WINDOW_BINARY="bumi_window_test"

# Compiler and flags
CXX="g++"
CXXFLAGS="-g -O2 -I$INCLUDE_DIR"
LDFLAGS="-lX11 -lGL"

# Source files
MAIN_SOURCES="$SRC_DIR/ventor/bumi_sysvideo.c $SRC_DIR/main.cpp"
TEST_WINDOW_SOURCES="$SRC_DIR/ventor/bumi_sysvideo.c $TEST_DIR/bumi_window_test.cpp"

# Function to print colored messages
print_message() {
    local color=$1
    local message=$2
    echo -e "${color}${message}${NC}"
}

# Check for dependencies
check_dependencies() {
    print_message "$YELLOW" "Checking dependencies..."
    local deps=("libx11-dev" "libgl1-mesa-dev" "g++")
    local missing=0

    for dep in "${deps[@]}"; do
        if ! dpkg -s "$dep" >/dev/null 2>&1; then
            print_message "$RED" "Error: $dep is not installed."
            missing=1
        else
            print_message "$GREEN" "$dep is installed."
        fi
    done

    if [ $missing -ne 0 ]; then
        print_message "$RED" "Please install missing dependencies (e.g., sudo apt-get install libx11-dev libgl1-mesa-dev g++)"
        exit 1
    fi

    # Check for xvfb for headless testing
    if command -v xvfb-run >/dev/null 2>&1; then
        print_message "$GREEN" "xvfb-run is installed (headless testing enabled)."
        XVFB="xvfb-run -a"
    else
        print_message "$YELLOW" "xvfb-run not found. Tests may fail without an X11 display."
        XVFB=""
    fi
}

# Clean build artifacts
clean() {
    print_message "$YELLOW" "Cleaning build artifacts..."
    if [ -d "$BIN_DIR" ]; then
        rm -rf "$BIN_DIR"
        print_message "$GREEN" "Cleaned $BIN_DIR."
    else
        print_message "$YELLOW" "Nothing to clean."
    fi
}

# Build the main program
build_main() {
    print_message "$YELLOW" "Creating bin directory..."
    mkdir -p "$BIN_DIR"

    print_message "$YELLOW" "Compiling main program..."
    if $CXX $CXXFLAGS $MAIN_SOURCES -o "$BIN_DIR/$MAIN_BINARY" $LDFLAGS; then
        print_message "$GREEN" "Main build successful: $MAIN_BINARY"
    else
        print_message "$RED" "Main build failed."
        exit 1
    fi
}

# Build the bumi_test_window program
build_test_window() {
    print_message "$YELLOW" "Creating bin directory..."
    mkdir -p "$BIN_DIR"

    print_message "$YELLOW" "Compiling $TEST_WINDOW_BINARY program..."
    if [ ! -f "$TEST_DIR/$TEST_WINDOW_BINARY.cpp" ]; then
        print_message "$RED" "Error: $TEST_DIR/$TEST_WINDOW_BINARY not found."
        exit 1
    fi
    if $CXX $CXXFLAGS $TEST_WINDOW_SOURCES -o "$BIN_DIR/$TEST_WINDOW_BINARY" $LDFLAGS; then
        print_message "$GREEN" "$TEST_WINDOW_BINARY build successful: $TEST_WINDOW_BINARY"
    else
        print_message "$RED" "$TEST_WINDOW_BINARY build failed."
        exit 1
    fi
}

# Run main tests
run_main_tests() {
    print_message "$YELLOW" "Running main tests..."
    if [ -f "$BIN_DIR/$MAIN_BINARY" ]; then
        print_message "$YELLOW" "Test 1: Running $MAIN_BINARY to check for crashes..."
        if $XVFB timeout 5s "$BIN_DIR/$MAIN_BINARY" >/dev/null 2>&1; then
            print_message "$GREEN" "Test 1 passed: Main binary runs without crashing."
        else
            print_message "$RED" "Test 1 failed: Main binary crashed or hung."
            exit 1
        fi
        print_message "$YELLOW" "Test 2: Please verify the window opens, shows a moving red rectangle, and responds to resizing and Escape key."
    else
        print_message "$RED" "Test failed: Main binary not found."
        exit 1
    fi
}

# Run test_window tests
run_test_window() {
    print_message "$YELLOW" "Running test_window..."
    if [ -f "$BIN_DIR/$TEST_WINDOW_BINARY" ]; then
        print_message "$YELLOW" "Running $TEST_WINDOW_BINARY..."
        if $XVFB timeout 5s "$BIN_DIR/$TEST_WINDOW_BINARY"; then
            print_message "$GREEN" "$TEST_WINDOW_BINARY passed: Check output for details."
        else
            print_message "$RED" "$TEST_WINDOW_BINARY failed: Check output for errors."
            exit 1
        fi
    else
        print_message "$RED" "Test failed: $TEST_WINDOW_BINARY binary not found."
        exit 1
    fi
}

# Main script logic
case "$1" in
    clean)
        clean
        ;;
    test_window)
        check_dependencies
        build_test_window
        run_test_window
        ;;
    *)
        check_dependencies
        build_main
        run_main_tests
        ;;
esac

exit 0