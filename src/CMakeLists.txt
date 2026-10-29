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

# Only the legacy unified tank renderer still needs a browser substitute.
# HUD orchestration and all modern visual passes compile from shared sources.
if(EMSCRIPTEN)
    list(FILTER SOURCES EXCLUDE REGEX "/rendering/TankRenderer\\.cpp$")
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

    # Stage level names containing '@' so file_packager never parses them as
    # source/destination delimiters. Include every shipped level and metadata.
    set(BROWSER_LEVEL_ASSETS "${CMAKE_CURRENT_BINARY_DIR}/browser-assets/levels")
    file(REMOVE_RECURSE "${BROWSER_LEVEL_ASSETS}")
    file(MAKE_DIRECTORY "${BROWSER_LEVEL_ASSETS}")
    file(GLOB BROWSER_LEVEL_FILES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/runtime/levels/*.txt"
        "${CMAKE_SOURCE_DIR}/runtime/levels/*.json")
    foreach(BROWSER_LEVEL_FILE IN LISTS BROWSER_LEVEL_FILES)
        get_filename_component(BROWSER_LEVEL_NAME "${BROWSER_LEVEL_FILE}" NAME)
        configure_file("${BROWSER_LEVEL_FILE}" "${BROWSER_LEVEL_ASSETS}/${BROWSER_LEVEL_NAME}" COPYONLY)
    endforeach()
    # Only codecs/assets actually referenced by SoundTask are needed.
    set(BROWSER_SOUND_ASSETS "${CMAKE_CURRENT_BINARY_DIR}/browser-assets/sounds")
    file(MAKE_DIRECTORY "${BROWSER_SOUND_ASSETS}")
    foreach(BROWSER_SOUND_FILE TGTest3.ogg cowbell.wav shatter.wav shatter2.wav
            hit3.wav shot.wav xphone.wav gain.wav jump.wav land.wav elastik.wav)
        configure_file("${CMAKE_SOURCE_DIR}/runtime/sounds/${BROWSER_SOUND_FILE}"
            "${BROWSER_SOUND_ASSETS}/${BROWSER_SOUND_FILE}" COPYONLY)
    endforeach()
    file(GLOB_RECURSE BROWSER_STATIC_ASSETS CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/runtime/fonts/*"
        "${CMAKE_SOURCE_DIR}/runtime/texture/*"
        "${BROWSER_SOUND_ASSETS}/*")
    set_property(TARGET ${EXECUTABLE_NAME} APPEND PROPERTY LINK_DEPENDS
        "${CMAKE_SOURCE_DIR}/web/shell.html" ${BROWSER_LEVEL_FILES} ${BROWSER_STATIC_ASSETS}
        "${CMAKE_SOURCE_DIR}/runtime/settings.txt"
        "${CMAKE_SOURCE_DIR}/runtime/nowbody.gsm" "${CMAKE_SOURCE_DIR}/runtime/nowturret.gsm"
        "${CMAKE_SOURCE_DIR}/runtime/cannon.gsm" "${CMAKE_SOURCE_DIR}/runtime/body.gsm")

    # Resource recovery uses C++ exceptions to report shader/upload failures.
    target_compile_options(${EXECUTABLE_NAME} PRIVATE "-fexceptions")
    target_link_options(${EXECUTABLE_NAME} PRIVATE
        "-fexceptions"
        "--shell-file=${CMAKE_SOURCE_DIR}/web/shell.html"
        "--preload-file=${BROWSER_SOUND_ASSETS}@/sounds"
        "--preload-file=${CMAKE_SOURCE_DIR}/runtime/fonts@/fonts"
        "--preload-file=${CMAKE_SOURCE_DIR}/runtime/texture@/texture"
        "--preload-file=${CMAKE_SOURCE_DIR}/runtime/settings.txt@/settings.txt"
        "--preload-file=${BROWSER_LEVEL_ASSETS}@/levels"
        "--preload-file=${CMAKE_SOURCE_DIR}/runtime/nowbody.gsm@/nowbody.gsm"
        "--preload-file=${CMAKE_SOURCE_DIR}/runtime/nowturret.gsm@/nowturret.gsm"
        "--preload-file=${CMAKE_SOURCE_DIR}/runtime/cannon.gsm@/cannon.gsm"
        "--preload-file=${CMAKE_SOURCE_DIR}/runtime/body.gsm@/body.gsm"
        "-sALLOW_MEMORY_GROWTH=1"
        # SceneData contains 128 KiB of terrain maps, and BuildScene also
        # needs a terrain temporary. The default 64 KiB WASM stack overflows
        # on the first frame; memory growth only grows the heap, not the stack.
        "-sSTACK_SIZE=1048576"
    )
endif()

# Include directories
target_include_directories(${EXECUTABLE_NAME} PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
)

# Keep the project's math.h available to quoted includes without allowing it
# to shadow the C library <math.h> used by Clang/GCC standard library headers.
if(CMAKE_CXX_COMPILER_ID MATCHES "^(AppleClang|Clang|GNU)$")
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
