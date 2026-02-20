cmake_minimum_required(VERSION 3.14)
project(TankGame)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directory to runtime folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/runtime)

# Find required packages
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(assimp REQUIRED)

# Try to find SDL2_mixer and SDL2_ttf
find_library(SDL2_MIXER_LIBRARY SDL2_mixer REQUIRED)
find_library(SDL2_TTF_LIBRARY SDL2_ttf REQUIRED)

# macOS specific OpenGL silence flag
if(APPLE)
    add_compile_definitions(GL_SILENCE_DEPRECATION)
endif()

# Build main executable
add_subdirectory(src)

# Enable testing and build tests
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()