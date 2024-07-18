{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    nixpkgs-open61541-1-3-10.url = "github:NixOS/nixpkgs/3f316d2a50699a78afe5e77ca486ad553169061e";
    nixpkgs-qt6-4-2.url = "github:NixOS/nixpkgs/194c2aa446b2b059886bb68be15ef6736d5a8c31";
    nixpkgs-gcc12-2-0.url = "github:NixOS/nixpkgs/e002d1926c23b29be7b59217e5f9740565b642ca";
    nixpkgs-llvm14-0-6.url = "github:NixOS/nixpkgs/7d0ed7f2e5aea07ab22ccb338d27fbe347ed2f11";
    nixpkgs-cmake3-25-1.url = "github:NixOS/nixpkgs/b8afc8489dc96f29f69bec50fdc51e27883f89c1";
  };

  outputs = {
    self,
    nixpkgs,
    ...
  } @ inputs: let
    systems = ["x86_64-linux" "x86_64-darwin"];

    mkPkgs = {
      system,
      additional_overlays ? [],
    }:
      import nixpkgs {
        inherit system;
        overlays =
          [
            (self: super: {
              open62541 = inputs.nixpkgs-open61541-1-3-10.legacyPackages.${system}.open62541.overrideAttrs {
                doCheck = !super.stdenv.isDarwin;
                meta.platforms = super.lib.platforms.linux ++ super.lib.platforms.darwin;
              };
            })
            (self: super: {open62541pp = super.callPackage ./nix/open62541pp.nix {};})
          ]
          ++ additional_overlays;
      };

    forSystem = system: f:
      f {
        inherit system;
        pkgs = mkPkgs {inherit system;};
        oldpkgs = mkPkgs {
          inherit system;
          additional_overlays = [
            (self: super: {
              qt6 = inputs.nixpkgs-qt6-4-2.legacyPackages.${system}.qt6;
              gcc12 = inputs.nixpkgs-gcc12-2-0.legacyPackages.${system}.gcc12;
              llvmPackages_14 = inputs.nixpkgs-llvm14-0-6.legacyPackages.${system}.llvmPackages_14;
              cmake = inputs.nixpkgs-cmake3-25-1.legacyPackages.${system}.cmake;
            })
          ];
        };
      };

    forAllSystems = f: nixpkgs.lib.genAttrs systems (system: forSystem system f);
  in {
    formatter = forAllSystems ({pkgs, ...}: pkgs.alejandra);

    packages = forAllSystems ({
      pkgs,
      oldpkgs,
      ...
    }:
      {
        default = pkgs.callPackage ./nix/magnesia.nix {};
      }
      // pkgs.lib.optionalAttrs pkgs.stdenv.isLinux {
        # This is for compatibility testing only. Don't use for main development or deployment.
        olddeps = oldpkgs.callPackage ./nix/magnesia.nix {stdenv = oldpkgs.gcc12Stdenv;};
      });

    devShells = forAllSystems (
      {
        pkgs,
        oldpkgs,
        system,
        ...
      }:
        {
          default = pkgs.mkShell {
            packages = with pkgs;
              [
                # faster incremental builds
                ninja

                # C++ static analysis/formatting
                llvmPackages_18.clang-tools
                llvmPackages_18.clang
                cppcheck
                include-what-you-use

                # CMake static analysis/formatting
                cmake-language-server
                cmake-format

                # doc generation
                doxygen
                graphviz
                sphinx
              ]
              # also C++ static analysis, but platform dependent
              ++ lib.optionals (lib.meta.availableOn stdenv.hostPlatform clazy) [clazy];

            inputsFrom = [self.packages.${system}.default];

            env = {
              QT_PLUGIN_PATH =
                "${pkgs.qt6.qtbase}/lib/qt-6/plugins"
                + pkgs.lib.optionalString pkgs.stdenv.isLinux ":${pkgs.qt6.qtwayland}/lib/qt-6/plugins";
            };
          };
        }
        // oldpkgs.lib.optionalAttrs oldpkgs.stdenv.isLinux {
          # This is for compatibility testing only. Don't use for main development.
          olddeps = oldpkgs.mkShell {
            packages = with oldpkgs; [
              ninja
              gcc12
              llvmPackages_14.clang
            ];
            inputsFrom = [self.packages.${system}.olddeps];
          };
        }
    );
  };
}
