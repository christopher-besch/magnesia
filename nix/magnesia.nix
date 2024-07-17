{
  stdenv,
  lib,
  cmake,
  qt6,
  open62541pp,
}:
stdenv.mkDerivation {
  pname = "magnesia";
  version = "trunk";

  src = ../.;

  nativeBuildInputs = [
    cmake
    qt6.wrapQtAppsHook
  ];

  buildInputs =
    [
      qt6.qtbase
      open62541pp
    ]
    ++ lib.optionals stdenv.isLinux [
      qt6.qtwayland
    ];

  cmakeFlags = [
    (lib.cmakeBool "MAGNESIA_BUILD_DOCS" false)
  ];

  # Qt's deploy script on macOS produces an app bundle that is independent of the nix store and wrapping fails
  dontWrapQtApps = stdenv.isDarwin;
}
