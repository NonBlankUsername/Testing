cmake_minimum_required(VERSION 3.10)

project(RobloxExecutor)

set(CMAKE_CXX_STANDARD 17)

# Add source files
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.hpp")

# Add executable
add_executable(RobloxExecutor ${SOURCES})

# Include directories
include_directories(src)
include_directories(${CMAKE_SOURCE_DIR}/third_party/pybind11/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/imgui)

# Link libraries
target_link_libraries(RobloxExecutor ${CMAKE_SOURCE_DIR}/third_party/pybind11/pybind11.lib)
target_link_libraries(RobloxExecutor ${CMAKE_SOURCE_DIR}/third_party/imgui/imgui.lib)