# Collect all source files recursively
file(GLOB_RECURSE SOURCES
    "*.cpp"
)

# Remove example files that shouldn't be compiled
list(FILTER SOURCES EXCLUDE REGEX ".*_example\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX "/rendering/BrowserDeferredRenderers\\.cpp$")

set(GAME_SOURCES ${SOURCES})
list(FILTER GAME_SOURCES EXCLUDE REGEX "/main\\.cpp$")
set(GAME_SOURCES ${GAME_SOURCES} PARENT_SCOPE)

# These renderers still use desktop compatibility OpenGL throughout their
# implementations.  The browser milestone deliberately links explicit
# deferred implementations instead of pulling unsupported APIs into WebGL.
if(EMSCRIPTEN)
    list(FILTER SOURCES EXCLUDE REGEX "/rendering/(EffectRenderer|EnemyTankRendererImpl|HUDRenderer|MenuRenderer|TankRenderer|TerrainRenderer)\\.cpp$")
    list(APPEND SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/rendering/BrowserDeferredRenderers.cpp"
    )
endif()

# Set executable name based on platform
if(APPLE)
    set(EXECUTABLE_NAME "tankgame-mac")
else()
    set(EXECUTABLE_NAME "tankgame-linux")
endif()

# Create the executable
add_executable(${EXECUTABLE_NAME} ${SOURCES})

if(EMSCRIPTEN)
    # An .html executable suffix asks Emscripten to emit its standard canvas
    # shell together with the JavaScript, WebAssembly, and preload data files.
    set_target_properties(${EXECUTABLE_NAME} PROPERTIES SUFFIX ".html")

    # file_packager uses '@' to separate host and virtual paths, while the
    # shipped level filenames themselves contain '@@'. Stage just the two
    # required levels in a directory so that delimiter is never ambiguous.
    set(BROWSER_LEVEL_ASSETS "${CMAKE_CURRENT_BINARY_DIR}/browser-assets/levels")
    file(MAKE_DIRECTORY "${BROWSER_LEVEL_ASSETS}")
    foreach(BROWSER_LEVEL_FILE
            "title@@.txt" "title@@.json" "level0@@.txt" "level0@@.json")
        configure_file(
            "${CMAKE_SOURCE_DIR}/runtime/levels/${BROWSER_LEVEL_FILE}"
            "${BROWSER_LEVEL_ASSETS}/${BROWSER_LEVEL_FILE}"
            COPYONLY
        )
    endforeach()

    target_link_options(${EXECUTABLE_NAME} PRIVATE
        "--preload-file=${CMAKE_SOURCE_DIR}/runtime/fonts@/fonts"
        "--preload-file=${CMAKE_SOURCE_DIR}/runtime/texture@/texture"
        "--preload-file=${CMAKE_SOURCE_DIR}/runtime/settings.txt@/settings.txt"
        "--preload-file=${BROWSER_LEVEL_ASSETS}@/levels"
        "--preload-file=${CMAKE_SOURCE_DIR}/runtime/nowbody.gsm@/nowbody.gsm"
        "--preload-file=${CMAKE_SOURCE_DIR}/runtime/nowturret.gsm@/nowturret.gsm"
        "--preload-file=${CMAKE_SOURCE_DIR}/runtime/cannon.gsm@/cannon.gsm"
        "--preload-file=${CMAKE_SOURCE_DIR}/runtime/body.gsm@/body.gsm"
        "-sALLOW_MEMORY_GROWTH=1"
    )
endif()

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
