# Enable testing
enable_testing()

# Add test executable
add_executable(sieve_tests
    test/test_sieve.cpp
)
target_link_libraries(sieve_tests sieve_lib)

# Register tests
add_test(NAME SieveTests COMMAND sieve_tests)
