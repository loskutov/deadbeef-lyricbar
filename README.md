[![Build Status](https://travis-ci.org/loskutov/deadbeef-lyricbar.svg)](https://travis-ci.org/loskutov/deadbeef-lyricbar)
# DeaDBeeF Lyricbar Plugin
A simple plugin for DeaDBeeF audio player that fetches and shows the song’s lyrics using track metadata and user-provided lyric fetchers.

![Screenshot](https://user-images.githubusercontent.com/1202012/51805459-90148b80-227e-11e9-9c0d-1df1d33fe1cd.png)


Inspired by [Infobar Plugin](https://bitbucket.org/dsimbiriatin/deadbeef-infobar/). If you need more functionality,
check [my fork of it](https://github.com/loskutov/deadbeef-infobar-ng), containing a few bug-fixes and minor improvements.

## Dependencies
To use this plugin, you need to have [gtkmm](http://www.gtkmm.org/) installed.
It is available in the repositories of most modern distributions (e.g. in Ubuntu you'll have to install `libgtkmm-3.0-dev` for the gtk3 version of lyricbar).

You need deadbeef.h file to build this plugin. The file /usr/include/deadbeef/deadbeef.h should've been installed with the player itself. If not -- look for deadbeef-plugin-dev package, or something like this. Or get the file from a source tarball.

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

In addition, if your tracks don't contain enough metadata, external lyrics providers can be used (see plugin preferences, the script launch command can use the whole [DeaDBeeF title formatting](https://github.com/DeaDBeeF-Player/deadbeef/wiki/Title-formatting-2.0) power, it's supposed to output the lyrics to stdout).
However, I recommend tools like [beets](https://github.com/beetbox/beets) to have the lyrics inside the tracks' metadata.

## Development Status
The plugin is now abandoned, as DeaDBeeF developers act agressively towards Russian-speaking users (see [the related commit](https://github.com/DeaDBeeF-Player/deadbeef/commit/d68495890fab7e3ac63674df72d8de82a592d78f)).
Besides, I haven't been actively developing it lately anyways.
Hopefully, the archived version of the plugin is usable enough, though.
