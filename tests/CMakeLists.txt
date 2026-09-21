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
    test_bullet_special.cpp
    test_main.cpp
    test_player.cpp
    test_tank_data_extractor.cpp
    test_player_manager_regression.cpp
    ${TEST_SOURCES}
)

# Include directories for tests
target_include_directories(tankgame_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${SDL2_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
)

# Link test executable with gtest and required libraries
target_link_libraries(tankgame_tests
    GTest::gtest
    GTest::gtest_main
    nlohmann_json::nlohmann_json
    ${SDL2_LIBRARIES}
    tankgame_sdl2_mixer
    ${SDL2_TTF_LIBRARY}
    ${OPENGL_LIBRARIES}
    ${CMAKE_DL_LIBS}
)

# Add GLU on Linux
if(UNIX AND NOT APPLE)
    find_library(GLU_LIBRARY GLU REQUIRED)
    target_link_libraries(tankgame_tests ${GLU_LIBRARY})
endif()

# Discover tests for CTest
include(GoogleTest)
gtest_discover_tests(tankgame_tests)
