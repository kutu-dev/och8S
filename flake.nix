{
  description = "A very basic flake";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs }:
    let
      allSystems = [
        "x86_64-linux"
        "aarch64-darwin"
      ];

      forAllSystems = fn:
        nixpkgs.lib.genAttrs allSystems
        (system: fn { pkgs = import nixpkgs { inherit system; }; });

    in {
      fmt = forAllSystems({ pkgs }: pkgs.alejandra);

      devShells = forAllSystems ({ pkgs }: {
        default = pkgs.mkShell {
          name = "och8S";
          nativeBuildInputs = with pkgs; [
            just
            gcc14
            meson
            ninja
            SDL2
            cppcheck
            clang-tools
            ripgrep
            glow
          ];
        };
      });
    };
}
