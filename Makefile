CFLAGS+=-Wall -fPIC -std=c99 -D_GNU_SOURCE
CXXFLAGS+=-Wall -O2 -fPIC -std=c++14
LIBFLAGS=`pkg-config --cflags libxml++-3.0 $(GTKMM) $(GTK)`
LIBS=`pkg-config --libs libxml++-3.0 $(GTKMM) $(GTK)`
prefix?=/usr

gtk3: GTKMM=gtkmm-3.0
gtk3: GTK=gtk+-3.0
gtk3: LYRICBAR=ddb_lyricbar_gtk3.so
gtk3: lyricbar

gtk2: GTKMM=gtkmm-2.4
gtk2: GTK=gtk+-2.0
gtk2: LYRICBAR=ddb_lyricbar_gtk2.so
gtk2: lyricbar

lyricbar: ui.o utils.o main.o
	$(if $(LYRICBAR),, $(error You should only access this target via "gtk3" or "gtk2"))
	$(CXX) -shared main.o ui.o utils.o -o $(LYRICBAR) $(LIBS)

ui.o: ui.cpp
	$(CXX) ui.cpp -c $(LIBFLAGS) $(CXXFLAGS)

utils.o: utils.cpp
	$(CXX) utils.cpp -c $(LIBFLAGS) $(CXXFLAGS)

main.o: main.c
	$(CC) $(CFLAGS) main.c -c `pkg-config --cflags $(GTK)`

install:
	cp *.so $(prefix)/lib/deadbeef/
	msgfmt gettext/ru/deadbeef-lyricbar.po -o /usr/share/locale/ru/LC_MESSAGES/deadbeef-lyricbar.mo

clean:
	rm -f *.o *.so

