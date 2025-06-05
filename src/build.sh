#!/bin/bash

EXECUTABLE_NAME="tankgame-linux"

echo "Starting build, please wait..."
g++ -std=c++14 -o $EXECUTABLE_NAME *.cpp -lSDL2 -lSDL2_mixer -lSDL2_ttf -lGL -lGLU -ldl -lassimp
mv ./$EXECUTABLE_NAME ../runtime/
echo "Build complete! Executable: ../runtime/$EXECUTABLE_NAME"
