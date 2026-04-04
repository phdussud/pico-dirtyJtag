{
  mkShell,
  autoreconfHook,
  fetchpatch,
  picotool,
  # one of these is not like the others
  picocom,
  picoDirtyJtagPkgs,
  usbutils,
  openocd,
}:

let
  openocd' = openocd.overrideAttrs (prev: {
    nativeBuildInputs = prev.nativeBuildInputs or [ ] ++ [ autoreconfHook ];
    patches = prev.patches or [ ] ++ [
      (fetchpatch {
        url = "https://github.com/numinit/openocd/commit/c4f139dcd15aeffda930c76c6d1c6bd9df9de658.patch";
        hash = "sha256-poxGWfhVDQF+Or0K2nvBC4YmJ9On2brBf/iurzPVTDE=";
      })
    ];
  });
in
mkShell {
  inputsFrom = [ picoDirtyJtagPkgs.rpi.pico ];
  nativeBuildInputs = [
    picotool
    picocom
    usbutils
    openocd'
  ];
}
