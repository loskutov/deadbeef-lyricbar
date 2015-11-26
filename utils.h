#pragma once
#ifndef LYRICBAR_UTILS_H
#define LYRICBAR_UTILS_H

#include <gtk/gtk.h>
#include <deadbeef/deadbeef.h>
#include <deadbeef/gtkui_api.h>
#include <libxml++/libxml++.h>
#include <libxml++/parsers/textreader.h>
#include "main.h"

struct pl_lock_guard {
    pl_lock_guard() { deadbeef->pl_lock(); }
    ~pl_lock_guard() { deadbeef->pl_unlock(); }
};

bool now_playing(DB_playItem_t *track);

void update_lyrics(void * tr);

#endif // LYRICBAR_UTILS_H
