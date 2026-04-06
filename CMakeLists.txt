cmake_minimum_required(VERSION 3.15)
project(sieve CXX)

# Generate compiler commands for IDEs and language servers.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Determine if we're building with Emscripten
if(EMSCRIPTEN)
    message(STATUS "Building with Emscripten for web target")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -sWASM=1")
else()
    message(STATUS "Building as native C++ project")
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Purpose-specific CMake modules
include(${CMAKE_SOURCE_DIR}/cmake/docs.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/format.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/lint.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/library_and_wasm.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/testing.cmake)

# Optional: Print build configuration
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
