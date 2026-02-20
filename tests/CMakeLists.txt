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
set(TEST_SOURCES
    ../src/Player.cpp
    ../src/Tank.cpp
    ../src/GameWorld.cpp
    ../src/Logger.cpp
    ../src/Camera.cpp
    ../src/Bullet.cpp
    ../src/Item.cpp
    ../src/FX.cpp
    ../src/GlobalTimer.cpp
    ../src/collision/CollisionSystem.cpp
    ../src/combat/CombatSystem.cpp
    ../src/TankCollisionHelper.cpp
    ../src/DisplayList.cpp
    ../src/TextureHandler.cpp
    ../src/LevelHandler.cpp
    ../src/TankHandler.cpp
    ../src/PlayerManager.cpp
    ../src/SoundTask.cpp
    ../src/InputHandlerFactory.cpp
    ../src/KeyboardMouseInputHandler.cpp
    ../src/GameCubeInputHandler.cpp
    ../src/GenericJoystickInputHandler.cpp
    ../src/TankTypeManager.cpp
    ../src/InputTask.cpp
    ../src/VideoTask.cpp
)

# Create test executable
add_executable(tankgame_tests
    test_main.cpp
    test_player.cpp
    ${TEST_SOURCES}
)

# Include directories for tests
target_include_directories(tankgame_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${SDL2_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
    ${ASSIMP_INCLUDE_DIRS}
)

# Link test executable with gtest and required libraries
target_link_libraries(tankgame_tests
    GTest::gtest
    GTest::gtest_main
    ${SDL2_LIBRARIES}
    ${SDL2_MIXER_LIBRARY}
    ${SDL2_TTF_LIBRARY}
    ${OPENGL_LIBRARIES}
    ${ASSIMP_LIBRARIES}
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
