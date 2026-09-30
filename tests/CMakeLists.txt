# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Enable testing
enable_testing()

# Collect source files needed for testing (exclude main.cpp)
set(TEST_SOURCES ${GAME_SOURCES})

# Create test executable
add_executable(tankgame_tests
    test_event_lifecycle.cpp
    test_geometry.cpp
    test_geometry_buffer.cpp
    test_graphics_capabilities.cpp
    test_matrix4.cpp
    test_material_texture.cpp
    test_bullet_special.cpp
    test_main.cpp
    test_player.cpp
    test_tank_data_extractor.cpp
    test_player_manager_regression.cpp
    test_resource_manager.cpp
    test_viewport_manager.cpp
    ${TEST_SOURCES}
)

# Include directories for tests
target_include_directories(tankgame_tests PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
)

# Keep the project's math.h available to quoted includes without allowing it
# to shadow the C library <math.h> used by Clang/GCC standard library headers.
if(CMAKE_CXX_COMPILER_ID MATCHES "^(AppleClang|Clang|GNU)$")
    target_compile_options(tankgame_tests PRIVATE
        "-iquote${CMAKE_SOURCE_DIR}/src"
    )
else()
    target_include_directories(tankgame_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )
endif()

# FindSDL2 can include SDL2main in this variable on Windows. Tests provide a
# normal main function, so link SDL2 itself without SDL2main's SDL_main shim.
set(TEST_SDL2_LIBRARIES ${SDL2_LIBRARIES})
list(FILTER TEST_SDL2_LIBRARIES EXCLUDE REGEX "SDL2main")

# Link test executable with gtest and required libraries
target_link_libraries(tankgame_tests
    GTest::gtest
    GTest::gtest_main
    nlohmann_json::nlohmann_json
    ${TEST_SDL2_LIBRARIES}
    tankgame_sdl2_mixer
    tankgame_sdl2_ttf
    ${OPENGL_LIBRARIES}
    ${CMAKE_DL_LIBS}
)

# Add GLU on native Linux/Unix desktops. Emscripten also reports UNIX, but
# WebAssembly targets must not discover or link the host desktop GLU library.
if(UNIX AND NOT APPLE AND NOT EMSCRIPTEN)
    find_library(GLU_LIBRARY GLU REQUIRED)
    target_link_libraries(tankgame_tests ${GLU_LIBRARY})
endif()

# Discover tests for CTest
include(GoogleTest)
gtest_discover_tests(tankgame_tests)
