{
  mkShell,
  picotool,
  # one of these is not like the others
  picocom,
  picoDirtyJtagPkgs,
  usbutils,
}:

mkShell {
  inputsFrom = [ picoDirtyJtagPkgs.rpi.pico ];
  nativeBuildInputs = [
    picotool
    picocom
    usbutils
  ];
}
