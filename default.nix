{
  lib,
  stdenv,
  cmake,
  pico-sdk,
  picotool,
  ninja,
  gcc-arm-embedded,
  python3,
  writeShellScriptBin,

  board ? "pico",
  boardType ? "BOARD_PICO",
  extraCFlags ? [ ],
}:

let
  pico-sdk' = pico-sdk.override {
    withSubmodules = true;
  };
in
stdenv.mkDerivation (finalAttrs: {
  pname = "dirtyjtag-rp2040";
  version = "0.1";

  src = lib.cleanSourceWith {
    filter = name: _: lib.match (baseNameOf (toString name)) "^.*\\.nix$" == null;
    src = lib.cleanSourceWith {
      filter = lib.cleanSourceFilter;
      src = ./.;
    };
  };

  nativeBuildInputs = [
    cmake
    ninja
    pico-sdk'
    picotool
    gcc-arm-embedded
    python3
  ];

  cmakeFlags = [
    "-DPICO_BOARD=${board}"
    "-DCMAKE_C_COMPILER=${finalAttrs.CC}"
    "-DCMAKE_CXX_COMPILER=${finalAttrs.CXX}"
  ];

  PICO_SDK_PATH = "${pico-sdk'}/lib/pico-sdk";

  NIX_HARDENING_DISABLE = true;

  CFLAGS = [ "-DBOARD_TYPE=${boardType}" ] ++ extraCFlags;
  CC = lib.getExe' gcc-arm-embedded "arm-none-eabi-gcc";
  CXX = lib.getExe' gcc-arm-embedded "arm-none-eabi-g++";

  installPhase = ''
    runHook preInstall
    mkdir -p $out
    cp *.{elf,uf2} $out/
    runHook postInstall
  '';

  dontFixup = true;

  passthru.flash = writeShellScriptBin "flash" ''
    ${lib.getExe picotool} load ${finalAttrs.finalPackage}/dirtyJtag.uf2
  '';
})
