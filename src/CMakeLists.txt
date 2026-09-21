# Collect all source files recursively
file(GLOB_RECURSE SOURCES
    "*.cpp"
)

# Remove example files that shouldn't be compiled
list(FILTER SOURCES EXCLUDE REGEX ".*_example\\.cpp$")

set(GAME_SOURCES ${SOURCES})
list(FILTER GAME_SOURCES EXCLUDE REGEX "/main\\.cpp$")
set(GAME_SOURCES ${GAME_SOURCES} PARENT_SCOPE)

# Set executable name based on platform
if(APPLE)
    set(EXECUTABLE_NAME "tankgame-mac")
else()
    set(EXECUTABLE_NAME "tankgame-linux")
endif()

# Create the executable
add_executable(${EXECUTABLE_NAME} ${SOURCES})

# Include directories
target_include_directories(${EXECUTABLE_NAME} PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
)

# Keep the project's math.h available to quoted includes without allowing it
# to shadow the C library <math.h> used by Emscripten's libc++ headers.
if(EMSCRIPTEN)
    target_compile_options(${EXECUTABLE_NAME} PRIVATE
        "-iquote${CMAKE_CURRENT_SOURCE_DIR}"
    )
else()
    target_include_directories(${EXECUTABLE_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()

# Link libraries
target_link_libraries(${EXECUTABLE_NAME}
    nlohmann_json::nlohmann_json
    ${SDL2_LIBRARIES}
    tankgame_sdl2_mixer
    tankgame_sdl2_ttf
    ${OPENGL_LIBRARIES}
    ${CMAKE_DL_LIBS}
)

# Add GLU on native Linux/Unix desktops. Emscripten also reports UNIX, but
# WebAssembly targets must not discover or link the host desktop GLU library.
if(UNIX AND NOT APPLE AND NOT EMSCRIPTEN)
    find_library(GLU_LIBRARY GLU REQUIRED)
    target_link_libraries(${EXECUTABLE_NAME} ${GLU_LIBRARY})
endif()
