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

# Documentation targets
add_subdirectory(docs)

# clang-format target
find_program(CLANG_FORMAT_EXE NAMES clang-format)
if(CLANG_FORMAT_EXE)
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXE} -i
            ${CMAKE_SOURCE_DIR}/include/bitset_index.h
            ${CMAKE_SOURCE_DIR}/src/bitset_index.cpp
            ${CMAKE_SOURCE_DIR}/src/wasm_bindings.cpp
            ${CMAKE_SOURCE_DIR}/test/test_sieve.cpp
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Formatting C++ sources with clang-format"
    )
else()
    message(WARNING "clang-format not found; 'format' target is unavailable")
endif()

# Library source files
set(SIEVE_SOURCES
    src/bitset_index.cpp
)

# Create library
add_library(sieve_lib ${SIEVE_SOURCES})

# Emscripten-specific library setup
if(EMSCRIPTEN)
    # Set up Emscripten export functions
    target_compile_options(sieve_lib PRIVATE -fPIC)
    
    # Create WebAssembly binding if needed
    add_executable(sieve_wasm src/wasm_bindings.cpp)
    target_link_libraries(sieve_wasm sieve_lib)
    
    # Emscripten link options
    set_target_properties(sieve_wasm PROPERTIES
        SUFFIX ".js"
        OUTPUT_NAME "sieve"
    )
    
    target_link_options(sieve_wasm PRIVATE
        -lembind
        -sALLOW_MEMORY_GROWTH=1
        -sEXPORT_ES6=1
        -sMODULARIZE=1
        -sEXPORTED_RUNTIME_METHODS=ccall,cwrap
    )
endif()

# Enable testing
enable_testing()

# Add test executable
add_executable(sieve_tests
    test/test_sieve.cpp
)
target_link_libraries(sieve_tests sieve_lib)

# Register tests
add_test(NAME SieveTests COMMAND sieve_tests)

# Optional: Print build configuration
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
