cmake_minimum_required(VERSION 3.14)
project(TankGame)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Use an installed package when available, with a pinned fallback so every
# toolchain (including Emscripten) receives the same CMake target.
find_package(nlohmann_json 3.11 QUIET)
if(NOT nlohmann_json_FOUND)
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

# Set output directory to runtime folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/runtime)

# Find required packages
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

# SDL2_mixer is a normal installed library on desktop platforms. Emscripten
# supplies it as a port, which must be enabled while compiling (for its
# headers) and linking. Only Ogg support is needed beyond SDL_mixer's WAV
# loader by the audio files that the game currently loads.
add_library(tankgame_sdl2_mixer INTERFACE)
if(EMSCRIPTEN)
    target_compile_options(tankgame_sdl2_mixer INTERFACE
        "-sUSE_SDL_MIXER=2"
    )
    target_link_options(tankgame_sdl2_mixer INTERFACE
        "-sUSE_SDL_MIXER=2"
        "-sSDL2_MIXER_FORMATS=[\"ogg\"]"
    )
else()
    find_library(SDL2_MIXER_LIBRARY SDL2_mixer REQUIRED)
    target_link_libraries(tankgame_sdl2_mixer INTERFACE
        ${SDL2_MIXER_LIBRARY}
    )
endif()

# SDL2_ttf follows the same dependency boundary as SDL2_mixer: desktop
# toolchains use their installed library, while Emscripten supplies a port
# whose compile and link settings must travel with every consumer.
add_library(tankgame_sdl2_ttf INTERFACE)
if(EMSCRIPTEN)
    target_compile_options(tankgame_sdl2_ttf INTERFACE
        "-sUSE_SDL_TTF=2"
    )
    target_link_options(tankgame_sdl2_ttf INTERFACE
        "-sUSE_SDL_TTF=2"
    )
else()
    find_library(SDL2_TTF_LIBRARY SDL2_ttf REQUIRED)
    target_link_libraries(tankgame_sdl2_ttf INTERFACE
        ${SDL2_TTF_LIBRARY}
    )
endif()

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
