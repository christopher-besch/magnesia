{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
  };

  outputs = {
    self,
    nixpkgs,
    ...
  }: let
    systems = ["x86_64-linux" "x86_64-darwin"];
    forAllSystems = f:
      nixpkgs.lib.genAttrs systems (system:
        f {
          inherit system;
          pkgs = import nixpkgs {
            inherit system;
            overlays = [
              (import ./nix/open62541.nix)
              (self: super: {open62541pp = super.callPackage ./nix/open62541pp.nix {};})
            ];
          };
        });
  in {
    formatter = forAllSystems ({pkgs, ...}: pkgs.alejandra);

    packages = forAllSystems ({pkgs, ...}: {
      default = pkgs.callPackage ./nix/magnesia.nix {};
      inherit (pkgs) open62541 open62541pp;
    });

    devShells = forAllSystems ({
      pkgs,
      system,
      ...
    }: {
      default = pkgs.mkShell {
        packages = with pkgs;
          [
            # faster incremental builds
            ninja

            # C++ static analysis/formatting
            llvmPackages_18.clang-tools
            cppcheck
            include-what-you-use

            # CMake static analysis/formatting
            cmake-language-server
            cmake-format

            # doc generation
            doxygen
            sphinx
          ]
          # also C++ static analysis, but platform dependent
          ++ lib.optionals (lib.meta.availableOn stdenv.hostPlatform clazy) [clazy];

        inputsFrom = [self.packages.${system}.default];
      };

      env = {
        QT_PLUGIN_PATH =
          "${pkgs.qt6.qtbase}/lib/qt-6/plugins"
          + pkgs.lib.optionalString pkgs.stdenv.isLinux ":${pkgs.qt6.qtwayland}/lib/qt-6/plugins";
      };
    });
  };
}
