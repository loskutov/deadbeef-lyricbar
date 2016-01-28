#pragma once
#ifndef LYRICBAR_UTILS_H
#define LYRICBAR_UTILS_H

#include <deadbeef/deadbeef.h>
#include <deadbeef/gtkui_api.h>
#include "main.h"
#ifndef __cplusplus
#include <stdbool.h>
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <libxml++/libxml++.h>
#pragma GCC diagnostic pop
#include <libxml++/parsers/textreader.h>
#include <experimental/optional>

struct pl_lock_guard {
    pl_lock_guard() { deadbeef->pl_lock(); }
    ~pl_lock_guard() { deadbeef->pl_unlock(); }
};

extern const DB_playItem_t * last;

bool is_playing(DB_playItem_t *track);

void update_lyrics(void * tr);
std::experimental::optional<std::string> get_lyrics_from_lyricwiki(DB_playItem_t * track);
extern "C" {
#endif // __cplusplus
int remove_from_cache_action(DB_plugin_action_t *, int ctx);
bool is_cached(const char * artist, const char * title);

#ifdef __cplusplus
}
#endif
#endif // LYRICBAR_UTILS_H
