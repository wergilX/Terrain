cmake_minimum_required(VERSION 3.31)
project(Terrain)

set(CMAKE_CXX_STANDARD 20)
set(EXECUTABLE_NAME Terrain)

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${SDL2_INCLUDE_DIRS}
        ${OPENGL_INCLUDE_DIRS}
        ${GLEW_INCLUDE_DIRS}
        ${GLM_INCLUDE_DIRS}
)

link_libraries(
        ${SDL2_LIBRARIES}
        ${OPENGL_LIBRARIES}
        ${GLEW_LIBRARIES}
)

add_executable(${EXECUTABLE_NAME} src/main.cpp)

add_subdirectory(src/core)
add_subdirectory(src/entities)
add_subdirectory(lib/imgui)