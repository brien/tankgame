#!/bin/bash

# Detect OS and set appropriate executable name and linking flags
OS=$(uname -s)
case $OS in
    Darwin)
        EXECUTABLE_NAME="tankgame-mac"
        OPENGL_FLAGS="-framework OpenGL"
        SILENCE_FLAGS="-DGL_SILENCE_DEPRECATION"
        echo "Building for macOS..."
        ;;
    Linux)
        EXECUTABLE_NAME="tankgame-linux"
        OPENGL_FLAGS="-lGL -lGLU"
        SILENCE_FLAGS=""
        echo "Building for Linux..."
        ;;
    *)
        echo "Unsupported OS: $OS"
        exit 1
        ;;
esac

echo "Starting build..."

echo 'Finding all .cpp files in current directory and subdirectories...'
CPP_FILES=$(find . -name '*.cpp' -type f)

echo 'Compiling the following files:'
echo "$CPP_FILES"
echo ''

g++ -std=c++14 -Wall -I/usr/local/include -I/usr/local/Cellar/nlohmann-json/3.12.0/include $SILENCE_FLAGS -o $EXECUTABLE_NAME $CPP_FILES -lSDL2 -lSDL2_mixer -lSDL2_ttf $OPENGL_FLAGS -ldl -lassimp

if [ $? -eq 0 ]; then
    mv ./$EXECUTABLE_NAME ../runtime/
    echo "Build complete! Executable: ../runtime/$EXECUTABLE_NAME"
else
    echo 'Build failed!'
    exit 1
fi
