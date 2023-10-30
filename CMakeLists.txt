cmake_minimum_required(VERSION 3.12)

project(amake)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)

# Source files
set(SOURCES
    main.cpp
)

# Executable
add_executable(amake ${SOURCES})

# Include directories
target_include_directories(amake PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link with libjsoncpp
target_link_libraries(amake PRIVATE yaml-cpp)

