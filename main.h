#pragma once
#ifndef LYRICBAR_MAIN_H
#define LYRICBAR_MAIN_H

#include <gtk/gtk.h>
#include <deadbeef/deadbeef.h>
#include <deadbeef/gtkui_api.h>

#ifdef __cplusplus
extern "C" {
#endif

extern DB_functions_t * deadbeef;

GtkWidget * construct_lyricbar();

int message_handler(struct ddb_gtkui_widget_s *, uint32_t id, uintptr_t ctx, uint32_t, uint32_t);

#ifdef __cplusplus
}
#endif

#endif // LYRICBAR_MAIN_H
