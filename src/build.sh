#!/bin/bash

EXECUTABLE_NAME="tankgame-linux"

echo "Starting build..."

echo 'Finding all .cpp files in current directory and subdirectories...'
CPP_FILES=$(find . -name '*.cpp' -type f)

echo 'Compiling the following files:'
echo "$CPP_FILES"
echo ''

g++ -std=c++14 -Wall -o $EXECUTABLE_NAME $CPP_FILES -lSDL2 -lSDL2_mixer -lSDL2_ttf -lGL -lGLU -ldl -lassimp

if [ $? -eq 0 ]; then
    mv ./$EXECUTABLE_NAME ../runtime/
    echo "Build complete! Executable: ../runtime/$EXECUTABLE_NAME"
else
    echo 'Build failed!'
    exit 1
fi
