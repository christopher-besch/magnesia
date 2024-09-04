{
  stdenv,
  lib,
  cmake,
  qt6,
  open62541pp,
  gtest,
}:
stdenv.mkDerivation (finalAttrs: {
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

  checkInputs = [
    gtest
  ];

  doCheck = true;

  cmakeFlags = [
    (lib.cmakeBool "MAGNESIA_BUILD_DOCS" false)
    (lib.cmakeBool "BUILD_TESTING" finalAttrs.doCheck)
  ];

  # Qt's deploy script on macOS produces an app bundle that is independent of the nix store and wrapping fails
  dontWrapQtApps = stdenv.isDarwin;
})
