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
