with import <nixpkgs> {}; {
  sdlEnv = stdenv.mkDerivation {
    name = "deadbeef-lyricbar";
    buildInputs = [
      stdenv
      pkgconfig
      gettext
      gnome3.gtkmm
      gnome3.gtk
      deadbeef
      libxmlxx3
    ];
  };
}
