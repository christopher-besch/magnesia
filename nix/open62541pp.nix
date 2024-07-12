{
  stdenv,
  lib,
  fetchFromGitHub,
  cmake,
  python3,
  open62541,
}:
stdenv.mkDerivation rec {
  pname = "open62541pp";
  version = "0.13.0";

  src = fetchFromGitHub {
    owner = "open62541pp";
    repo = "open62541pp";
    rev = "v${version}";
    sha256 = "sha256-Oyxwz+di9LKy9E7e3zuIghFFCKTAy4wf1kpRk1sbkzI=";
  };

  nativeBuildInputs = [
    cmake
  ];

  propagatedNativeBuildInputs = [
    # The cmake config file for open62541 depends on python.
    # This dependency is transitively passed through by open62541pp's config file.
    python3
  ];

  propagatedBuildInputs = [
    open62541
  ];

  cmakeFlags = [
    (lib.cmakeBool "BUILD_SHARED_LIBS" (!stdenv.hostPlatform.isStatic))
    (lib.cmakeBool "UAPP_BUILD_EXAMPLES" false)
    (lib.cmakeBool "UAPP_BUILD_TESTS" false)
    (lib.cmakeBool "UAPP_INTERNAL_OPEN62541" false)
  ];
}
