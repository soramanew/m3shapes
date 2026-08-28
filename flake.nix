{
  description = "A Qt6 library for Material 3 Expressive shapes";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
  };

  outputs = {
    self,
    nixpkgs,
    ...
  }: let
    inherit (nixpkgs.lib) genAttrs platforms lists systems;

    pkgsOf = nixpkgs.legacyPackages;
    systems' = lists.intersectLists platforms.linux systems.flakeExposed;
    eachSystem = genAttrs systems';
  in {
    formatter = eachSystem (system: pkgsOf.${system}.alejandra);

    packages = eachSystem (system: let
      pkgs = pkgsOf.${system};
    in rec {
      m3shapes = pkgs.callPackage ./nix {
        stdenv = pkgs.clangStdenv;
      };
      debug = m3shapes.override {debug = true;};
      default = m3shapes;
    });

    devShells = eachSystem (system: {
      default = let
        pkgs = pkgsOf.${system};
        m3shapes = self.packages.${system}.m3shapes;
        mkShell = pkgs.mkShell.override {stdenv = m3shapes.stdenv;};
      in
        mkShell {
          inputsFrom = [m3shapes];
          packages = with pkgs; [clazy clang-tools];
        };
    });
  };
}
