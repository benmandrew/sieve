{
  description = "sieve - bitset-based Wordle filtering engine";
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };
  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
        docsPython = pkgs.python3.withPackages (ps: [
          ps.sphinx
          ps.breathe
          ps.furo
        ]);
      in {
        packages = {
          default = pkgs.stdenv.mkDerivation {
            pname = "sieve";
            version = "0.1.0";
            src = ./.;
            nativeBuildInputs = [
              pkgs.cmake
              pkgs.ninja
            ];
            cmakeFlags = [
              "-DCMAKE_BUILD_TYPE=Release"
            ];
            doCheck = true;
            checkPhase = ''
              ctest --output-on-failure
            '';
            installPhase = ''
              mkdir -p "$out/lib" "$out/include"
              cp libsieve_lib.a "$out/lib/"
              cp -r ../include/. "$out/include/"
            '';
          };
          docs = pkgs.runCommand "sieve-docs-0.1.0"
            {
              src = ./.;
              nativeBuildInputs = [
                pkgs.cmake
                pkgs.ninja
                pkgs.clang
                pkgs.doxygen
                docsPython
              ];
            }
            ''
              export IN_NIX_SHELL=1
              export CC=clang
              export CXX=clang++
              cmake -S "$src" -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
              cmake --build build --target docs
              mkdir -p "$out"
              cp -r build/docs/html "$out/html"
            '';
        };
        devShells.default = pkgs.mkShell {
          packages = [
            pkgs.cmake
            pkgs.ninja
            pkgs.gcc
            pkgs.clang
            pkgs.clang-tools
            pkgs.cppcheck
            pkgs.doxygen
            pkgs.flake-checker
            docsPython
            pkgs.cpplint
          ];
          shellHook = ''
            echo "sieve dev shell ready"
            echo "Try: cmake -S . -B build && cmake --build build --target format-ci lint docs"
          '';
        };
        apps.check = {
          type = "app";
          program = toString ((pkgs.writeShellApplication {
            name = "sieve-check";
            runtimeInputs = [
              pkgs.cmake
              pkgs.ninja
              pkgs.clang-tools
              pkgs.cppcheck
              pkgs.doxygen
              pkgs.flake-checker
              docsPython
              pkgs.cpplint
            ];
            text = ''
              set -euo pipefail
              cmake -S . -B build
              cmake --build build --target sieve_tests format-ci flake-check lint docs
              ctest --test-dir build --output-on-failure
            '';
          }) + "/bin/sieve-check");
        };
      });
}
