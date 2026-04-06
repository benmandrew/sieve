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
