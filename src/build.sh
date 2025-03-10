 #!/bin/bash
g++ -o tankgame-linux *.cpp -lSDL2 -lSDL2_mixer -lSDL2_ttf -lGL -lGLU -ldl -lassimp
mv ./tankgame-linux ../runtime/
echo "Build complete! Executable in ../runtime/"
