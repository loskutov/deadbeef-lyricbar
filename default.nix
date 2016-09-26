with import <nixpkgs> {}; {
  sdlEnv = stdenv.mkDerivation {
    name = "deadbeef-lyricbar";
    buildInputs = [
      stdenv
      pkgconfig
      gnome3.gtkmm
      gnome3.gtk
      deadbeef
      libxmlxx3
    ];
  };
}
