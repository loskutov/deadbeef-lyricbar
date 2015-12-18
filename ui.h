#pragma once
#ifndef LYRICBAR_UI_H
#define LYRICBAR_UI_H
#include <gtk/gtk.h>
#include <deadbeef/deadbeef.h>

#ifdef __cplusplus

#include <glibmm/ustring.h>

void set_lyrics(DB_playItem_t * track, const Glib::ustring & lyrics);

extern "C" {
#endif

GtkWidget* construct_lyricbar();

int message_handler(struct ddb_gtkui_widget_s *, uint32_t id, uintptr_t ctx, uint32_t, uint32_t);

void lyricbar_destroy();

#ifdef __cplusplus
}
#endif

#endif // LYRICBAR_UI_H
