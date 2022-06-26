#pragma once
#ifndef LYRICBAR_UTILS_H
#define LYRICBAR_UTILS_H

#include <gtk/gtk.h>
#include <deadbeef/deadbeef.h>
#include <deadbeef/gtkui_api.h>

#ifndef __cplusplus
#include <stdbool.h>
#else
#include <glibmm/main.h>
#include <experimental/optional>

#include "main.h"

struct pl_lock_guard {
	pl_lock_guard() { deadbeef->pl_lock(); }
	~pl_lock_guard() { deadbeef->pl_unlock(); }
};

struct id3v2_tag {
	DB_id3v2_tag_t tag{};
	~id3v2_tag() { deadbeef->junk_id3v2_free(&tag); }
};

extern const DB_playItem_t *last;

bool is_playing(DB_playItem_t *track);

void update_lyrics(void *tr);

std::experimental::optional<Glib::ustring> download_lyrics_from_lyricwiki(DB_playItem_t *track);
std::experimental::optional<Glib::ustring> get_lyrics_from_script(DB_playItem_t *track);

int mkpath(const std::string &name, mode_t mode);

extern "C" {
#endif // __cplusplus
int remove_from_cache_action(DB_plugin_action_t *, int ctx);
bool is_cached(const char *artist, const char *title);
void ensure_lyrics_path_exists();

#ifdef __cplusplus
}
#endif
#endif // LYRICBAR_UTILS_H
