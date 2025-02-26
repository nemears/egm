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

    packages.x86_64-linux = {
      egm = pkgs.stdenv.mkDerivation {
        name = "egm";
        src = ./.;
        nativeBuildInputs = with pkgs; [ pkg-config ];
        installPhase = ''
          mkdir -p $out/lib/pkgconfig $out/include
          cp -r $src/include/egm $out/include
          echo "prefix=$out
          
          Name: egm
          Description: generic object pool manager
          Version: 0.0.1
          Cflags: -I$out/include" > $out/lib/pkgconfig/egm.pc
        '';
      };
      
      default = self.packages.x86_64-linux.egm;
    };
  };
}
