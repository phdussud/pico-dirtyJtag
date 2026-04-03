{
  description = "dirtyjtag for the pi pico, nixified";
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-parts.url = "github:hercules-ci/flake-parts";
  };

  outputs =
    inputs@{
      self,
      flake-parts,
      ...
    }:
    flake-parts.lib.mkFlake { inherit inputs; } {
      imports = [
        inputs.flake-parts.flakeModules.easyOverlay
      ];

      systems = [
        "x86_64-linux"
        "aarch64-linux"
      ];

      perSystem =
        {
          config,
          system,
          inputs',
          pkgs,
          final,
          ...
        }:
        {
          _module.args.pkgs = import inputs.nixpkgs {
            inherit system;
            overlays = [
              self.overlays.default
            ];
          };

          overlayAttrs = {
            picoDirtyJtagPkgs =
              let
                default = final.callPackage ./default.nix { };
              in
              {
                rpi.pico = default;
                adafruit.itsy = default.override {
                  # not tested but should work
                  board = "adafruit_itsybitsy_rp2040";
                  boardType = "BOARD_ADAFRUIT_ITSY";
                };
                adafruit.kb2040 = default.override {
                  board = "adafruit_kb2040";
                  boardType = "BOARD_ADAFRUIT_ITSY";
                  extraCFlags = [ "-DBOARD_ADAFRUIT_ITSY_KB2040" ];
                };
                spoke.rp2040 = default.override {
                  # not tested
                  boardType = "BOARD_SPOKE_RP2040";
                };
                machdyne.werkzeug = default.override {
                  # not tested
                  board = "machdyne_werkzeug";
                  boardType = "BOARD_WERKZEUG";
                };
                qmtech.rp2040-daughterboard = default.override {
                  # not tested
                  boardType = "BOARD_QMTECH_RP2040_DAUGHTERBOARD";
                };
                waveshare.rp2040-zero = default.override {
                  board = "waveshare_rp2040_zero";
                  boardType = "BOARD_RP2040_ZERO";
                };
              };
          };

          packages =
            let
              inherit (pkgs.picoDirtyJtagPkgs)
                rpi
                adafruit
                spoke
                machdyne
                qmtech
                waveshare
                ;
            in
            {
              default = rpi.pico;
              rpi-pico = rpi.pico;
              adafruit-itsy = adafruit.itsy;
              adafruit-kb2040 = adafruit.kb2040;
              spoke-rp2040 = spoke.rp2040;
              machdyne-werkzeug = machdyne.werkzeug;
              qmtech-rp2040-daughterboard = qmtech.rp2040-daughterboard;
              waveshare-rp2040-zero = waveshare.rp2040-zero;
            };

          devShells = {
            default = final.callPackage ./shell.nix { };
          };
        };
    };
}
