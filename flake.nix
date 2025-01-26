{
  description = "A very basic flake";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs = { self, nixpkgs }: 
  let
    pkgs = nixpkgs.legacyPackages.x86_64-linux;
  in
  {

    devShells.x86_64-linux.default = pkgs.mkShell {
      packages = with pkgs; [
        #build
        meson
        pkg-config
        ninja
        
        #dependencies
        gtest
        yaml-cpp

        #debugging
        gdb
        valgrind
      ];
    };

    packages.x86_64-linux.hello = nixpkgs.legacyPackages.x86_64-linux.hello;

    packages.x86_64-linux.default = self.packages.x86_64-linux.hello;

  };
}
