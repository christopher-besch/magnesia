self: super: {
  open62541 = super.open62541.overrideAttrs rec {
    version = "1.3.11";
    src = super.fetchFromGitHub {
      owner = "open62541";
      repo = "open62541";
      rev = "v${version}";
      sha256 = "sha256-JOrobggqedLGQEaIPkxtR9SlS49bXHGRO4KWivrhelA=";
      fetchSubmodules = true;
    };

    # See https://github.com/NixOS/nixpkgs/issues/144170
    # This was fixed upstream in open62541 v1.4.0
    patches = [
      (super.fetchpatch {
        name = "Ensure-absolute-paths-in-pkg-config-file.patch";
        url = "https://github.com/open62541/open62541/commit/023d4b6b8bdec987f8f3ffee6c09801bbee4fa2d.patch";
        sha256 = "sha256-mq4h32js2RjI0Ljown/01SXA3gc+7+zX8meIcvDPvoA=";
      })
    ];

    meta = {
      # nixpkgs only claims linux
      platforms = super.lib.platforms.linux ++ super.lib.platforms.darwin;
    };
  };
}
