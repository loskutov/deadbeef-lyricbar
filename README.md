[![Build Status](https://travis-ci.org/loskutov/deadbeef-lyricbar.svg)](https://travis-ci.org/loskutov/deadbeef-lyricbar)
# DeaDBeeF Lyricbar Plugin
A simple plugin for DeaDBeeF audio player that fetches and shows the song’s lyrics using LyricWiki.

![Screenshot](https://user-images.githubusercontent.com/1202012/51805459-90148b80-227e-11e9-9c0d-1df1d33fe1cd.png)


Inspired by [Infobar Plugin](https://bitbucket.org/dsimbiriatin/deadbeef-infobar/). If you need more functionality,
check [my fork of it](https://bitbucket.org/IgnatLoskutov/deadbeef-infobar-ng), containing a few bug-fixes and minor improvements.

## Dependencies
To use this plugin, you need to have [gtkmm](http://www.gtkmm.org/) and [libxml++ 3](http://libxmlplusplus.sourceforge.net/) installed.

While gtkmm is available in the repositories of most modern distributions (e.g. in Ubuntu you'll have to install `libgtkmm-3.0-dev` for the gtk3 version of lyricbar), libxml++3 is absent in many of them. If that's the case, you'll have to build it from sources (e.g. for Ubuntu: `sudo apt install checkinstall libxml2-dev && wget http://ftp.gnome.org/pub/GNOME/sources/libxml++/3.0/libxml++-3.0.1.tar.xz && tar -xJf libxml++-3.0.1.tar.xz && cd libxml++-3.0.1 && ./configure --prefix=/usr && make && sudo checkinstall`).

## Installation
Clone this repository and perform the following:
```sh
make [gtk2 or gtk3]
sudo cp *.so /usr/lib/deadbeef # depends on where deadbeef is installed
# OR, to install for the current user only
mkdir -p ~/.local/lib/deadbeef && cp *.so ~/.local/lib/deadbeef
```

## Usage
Activate Design Mode (View → Design mode) and add Lyricbar somewhere. Disable Design Mode back and enjoy the music :)
