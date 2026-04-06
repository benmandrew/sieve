# clang-format target
find_program(CLANG_FORMAT_EXE NAMES clang-format)
if(CLANG_FORMAT_EXE)
    set(FORMAT_SOURCES
        ${CMAKE_SOURCE_DIR}/include/bitset_index.h
        ${CMAKE_SOURCE_DIR}/src/bitset_index.cpp
        ${CMAKE_SOURCE_DIR}/src/wasm_bindings.cpp
        ${CMAKE_SOURCE_DIR}/test/test_sieve.cpp
    )

    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXE} -i
            ${FORMAT_SOURCES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Formatting C++ sources with clang-format"
    )

    add_custom_target(format-ci
        COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror
            ${FORMAT_SOURCES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Checking C++ formatting with clang-format"
    )
else()
    message(WARNING "clang-format not found; 'format' and 'format-ci' targets are unavailable")
endif()
