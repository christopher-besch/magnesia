{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    nixpkgs-open61541-1-3-10.url = "github:NixOS/nixpkgs/3f316d2a50699a78afe5e77ca486ad553169061e";
  };

  outputs = {
    self,
    nixpkgs,
    ...
  } @ inputs: let
    systems = ["x86_64-linux" "x86_64-darwin"];

    forSystem = system: f:
      f {
        inherit system;
        pkgs = import nixpkgs {
          inherit system;
          overlays = [
            (self: super: {
              open62541 =
                (inputs.nixpkgs-open61541-1-3-10.legacyPackages.${system}.open62541.overrideAttrs {
                  doCheck = !super.stdenv.isDarwin;
                  meta.platforms = super.lib.platforms.linux ++ super.lib.platforms.darwin;
                })
                .override {withEncryption = "openssl";};
              open62541pp = self.callPackage ./nix/open62541pp.nix {};
            })
          ];
        };
      };

    forAllSystems = f: nixpkgs.lib.genAttrs systems (system: forSystem system f);
  in {
    formatter = forAllSystems ({pkgs, ...}: pkgs.alejandra);

    packages = forAllSystems (
      {pkgs, ...}: {default = pkgs.callPackage ./nix/magnesia.nix {};}
    );

    devShells = forAllSystems (
      {
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
              llvmPackages_19.clang-tools
              llvmPackages_19.clang
              cppcheck

              # CMake static analysis/formatting
              cmake-language-server
              cmake-format

              # doc generation
              doxygen
              graphviz
              sphinx

              # spell checking
              codespell

              # shell
              shellcheck
              shfmt

              sqlite-interactive
              parallel
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
    );
  };
}
