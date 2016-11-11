with import <nixpkgs> {}; {
  lyricbar = stdenv.mkDerivation {
    name = "deadbeef-lyricbar";
    buildInputs = [
      stdenv
      pkgconfig
      gettext
      gnome2.gtkmm
      gnome3.gtkmm
      gnome2.gtk
      gnome3.gtk
      deadbeef
      libxmlxx3
    ];
  };
}
