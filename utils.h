#pragma once
#ifndef LYRICBAR_UTILS_H
#define LYRICBAR_UTILS_H

#include <gtk/gtk.h>
#include <deadbeef/deadbeef.h>
#include <deadbeef/gtkui_api.h>
#include "main.h"
#ifndef __cplusplus
#include <stdbool.h>
#else
#include <libxml++/libxml++.h>
#include <libxml++/parsers/textreader.h>

struct pl_lock_guard {
    pl_lock_guard() { deadbeef->pl_lock(); }
    ~pl_lock_guard() { deadbeef->pl_unlock(); }
};

bool is_playing(DB_playItem_t *track);

void update_lyrics(void * tr);
extern "C" {
#endif // __cplusplus
int remove_from_cache_action(DB_plugin_action_t *action, int ctx);
bool is_cached(const char * artist, const char * title);

#ifdef __cplusplus
}
#endif
#endif // LYRICBAR_UTILS_H
