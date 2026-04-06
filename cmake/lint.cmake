# Static analysis and lint targets
set(LINT_TARGETS)

find_package(Python3 COMPONENTS Interpreter QUIET)
find_program(CPPLINT_EXE NAMES cpplint)
if(Python3_Interpreter_FOUND AND CPPLINT_EXE)
    add_custom_target(lint-cpplint
        COMMAND ${CPPLINT_EXE}
            --quiet
            --filter=-whitespace/indent,-whitespace/indent_namespace
            ${CMAKE_SOURCE_DIR}/include/bitset_index.h
            ${CMAKE_SOURCE_DIR}/src/bitset_index.cpp
            ${CMAKE_SOURCE_DIR}/src/wasm_bindings.cpp
            ${CMAKE_SOURCE_DIR}/test/test_sieve.cpp
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running cpplint"
    )
    list(APPEND LINT_TARGETS lint-cpplint)
else()
    add_custom_target(lint-cpplint
        COMMAND ${CMAKE_COMMAND} -E echo "lint-cpplint requires Python 3 and cpplint in PATH."
        COMMENT "cpplint unavailable"
    )
endif()

find_program(CLANG_TIDY_EXE NAMES clang-tidy)
if(CLANG_TIDY_EXE)
    add_custom_target(lint-clang-tidy
        COMMAND ${CLANG_TIDY_EXE}
            -p ${CMAKE_BINARY_DIR}
            --checks=-*,clang-analyzer-*,bugprone-*,performance-*
            ${CMAKE_SOURCE_DIR}/src/bitset_index.cpp
            ${CMAKE_SOURCE_DIR}/test/test_sieve.cpp
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-tidy"
    )
else()
    add_custom_target(lint-clang-tidy
        COMMAND ${CMAKE_COMMAND} -E echo "lint-clang-tidy requires clang-tidy in PATH."
        COMMENT "clang-tidy unavailable"
    )
endif()

find_program(CPPCHECK_EXE NAMES cppcheck)
if(CPPCHECK_EXE)
    add_custom_target(lint-cppcheck
        COMMAND ${CPPCHECK_EXE}
            --quiet
            --std=c++17
            --language=c++
            --enable=warning,style,performance,portability
            --suppress=missingIncludeSystem
            -I ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_SOURCE_DIR}/test
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running cppcheck"
    )
    list(APPEND LINT_TARGETS lint-cppcheck)
else()
    add_custom_target(lint-cppcheck
        COMMAND ${CMAKE_COMMAND} -E echo "lint-cppcheck requires cppcheck in PATH."
        COMMENT "cppcheck unavailable"
    )
endif()

if(LINT_TARGETS)
    add_custom_target(lint)
    add_dependencies(lint ${LINT_TARGETS})
else()
    add_custom_target(lint
        COMMAND ${CMAKE_COMMAND} -E echo "No lint tools available. Install cpplint, clang-tidy, and/or cppcheck."
        COMMENT "Lint tooling unavailable"
    )
endif()
