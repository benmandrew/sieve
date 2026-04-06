# Nix/flake-specific checks
find_program(FLAKE_CHECKER_EXE NAMES flake-checker)
if(FLAKE_CHECKER_EXE)
    add_custom_target(flake-check
        COMMAND ${FLAKE_CHECKER_EXE}
            --check-outdated
            --check-owner
            --check-supported
            --fail-mode
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running flake-checker on flake.lock"
    )
else()
    add_custom_target(flake-check
        COMMAND ${CMAKE_COMMAND} -E echo "flake-checker not found in PATH. Install it via nix develop or nix shell nixpkgs#flake-checker."
        COMMAND ${CMAKE_COMMAND} -E false
        COMMENT "flake-checker unavailable"
    )
endif()
