[![Build Status](https://travis-ci.org/loskutov/deadbeef-lyricbar.svg)](https://travis-ci.org/loskutov/deadbeef-lyricbar)
# DeaDBeeF Lyricbar Plugin
A simple plugin for DeaDBeeF audio player that fetches and shows the song’s lyrics.
![Screenshot](http://rain.ifmo.ru/~loskutov/deadbeef-lyricbar.png)

Inspired by [Infobar Plugin](https://bitbucket.org/dsimbiriatin/deadbeef-infobar/). If you need more functionality,
check [my fork of it](https://bitbucket.org/IgnatLoskutov/deadbeef-infobar-ng), containing a few bug-fixes and minor improvements.

## Dependencies
To use this plugin, you need to have [gtkmm](http://www.gtkmm.org/) and [libxml++](http://libxmlplusplus.sourceforge.net/) installed (both available in the repositories of most modern distributions).

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
