#pragma once
#include <gtk/gtk.h>
#include <deadbeef/deadbeef.h>
#include <deadbeef/gtkui_api.h>
#include <libxml++/libxml++.h>
#include <libxml++/parsers/textreader.h>

//int dl_file(CURL * curl, const std::string & url, std::string & result);
bool now_playing(DB_playItem_t *track);

void update_lyrics(void * tr);
