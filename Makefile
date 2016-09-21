CXXFLAGS+=-Wall -O2 -fPIC -std=c++14
LIBFLAGS=`pkg-config --cflags gtkmm-3.0 libxml++-3.0`
LIBS=`pkg-config --libs gtkmm-3.0 libxml++-3.0`

all: ui.o utils.o main.o
	gcc -shared main.o ui.o utils.o -o ddb_lyricbar_gtk3.so $(LIBS)
ui.o: ui.cpp
	g++ $(CXXFLAGS) ui.cpp -c $(LIBFLAGS)
utils.o: utils.cpp
	g++ $(CXXFLAGS) utils.cpp -c $(LIBFLAGS)
main.o: main.c
	gcc -Wall -fPIC -std=c11 -D_GNU_SOURCE main.c -c `pkg-config --cflags gtkmm-3.0`
install:
	cp *.so /usr/lib/deadbeef/
	msgfmt gettext/ru/deadbeef-lyricbar.po -o /usr/share/locale/ru/LC_MESSAGES/deadbeef-lyricbar.mo
clean:
	rm -f *.o *.so
