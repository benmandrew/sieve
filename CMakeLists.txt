cmake_minimum_required(VERSION 3.15)
project(sieve CXX)

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

# Library source files
set(SIEVE_SOURCES
    src/sieve.cpp
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
