with import (builtins.fetchTarball {
  url = "https://github.com/NixOS/nixpkgs/archive/release-22.05.tar.gz";
  sha256 = "sha256:154x9swf494mqwi4z8nbq2f0sp8pwp4fvx51lqzindjfbb9yxxv5";
}) { };

stdenv.mkDerivation rec {
  name = "wiztoolkit";
  src = ./.;

  logging = fetchgit {
    url = "https://github.com/stealthsoftwareinc/stealth_logging.git";
    sha256 = "sha256-NefO3x4yMN8PKxd6oE1M1WyWjIvLxw3Jh93mZ/ezUrE=";
  };

  sst = fetchgit {
    url = "https://github.com/stealthsoftwareinc/sst.git";
    rev = "e5cb49bc8c6f4d95f54cffc89c42d459b7caa3e";
    # Must match `SSH_COMMIT` in `sst_bignum_only.sh`.
    sha256 = "sha256-kRYrBBgb9Aclh5X/arNhwaacF3H8Tdrsyq7f/KQslB0=";
  };

  patches = [ ./nix/cmakelists-external-deps.patch ./nix/sst-no-wget.patch ];

  dontConfigure = true;

  preBuild = ''
    set -e
    patchShebangs src/deps/

    ln -s ${logging} src/deps/logging
    ln -s ${sst} src/deps/sst
    mkdir -p src/deps/sst_bignum
    pushd src/deps/sst_bignum
    ../sst_bignum_only.sh
    popd

    echo 'success' > src/deps/deps.success
  '';

  buildInputs = [ cmake flatbuffers gtest openssl python3 ];

  makeFlags = if stdenv.isDarwin then [ "CXX=clang++" ] else [ "CXX=g++" ];
  buildFlagsArray =
    [ "FLATC_TOOL=${flatbuffers}/bin/flatc --cpp --cpp-std c++11" ];

  installPhase = ''
    mkdir -p $out/bin
    cp target/build/src/main/cpp/wtk-* $out/bin/
  '';
}
