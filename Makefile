# Infobar plugin for DeaDBeeF music player
# Copyright (C) 2011-2012 Dmitriy Simbiriatin <dmitriy.simbiriatin@gmail.com>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

CXXFLAGS+=-Wall -O2 -fPIC -std=c++14
LIBFLAGS=`pkg-config --cflags gtkmm-2.4 libxml++-2.6`
LIBS=`pkg-config --libs gtkmm-2.4 libcurl libxml++-2.6`

all:
	gcc -Wall -fPIC -std=c11 -D_GNU_SOURCE main.c -c `pkg-config --cflags gtkmm-2.4`
	g++ $(CXXFLAGS) ui.cpp -c $(LIBFLAGS)
	g++ $(CXXFLAGS) utils.cpp -c $(LIBFLAGS)
	gcc -shared main.o ui.o utils.o -o ddb_lyrics.so $(LIBS)
install:
	cp *.so /usr/lib/deadbeef/

