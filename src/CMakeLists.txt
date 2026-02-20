# Collect all source files recursively
file(GLOB_RECURSE SOURCES
    "*.cpp"
)

# Remove example files that shouldn't be compiled
list(FILTER SOURCES EXCLUDE REGEX ".*_example\\.cpp$")

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
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SDL2_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
    ${ASSIMP_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(${EXECUTABLE_NAME}
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
    target_link_libraries(${EXECUTABLE_NAME} ${GLU_LIBRARY})
endif()