#include <gtk/gtk.h>
#include <deadbeef/deadbeef.h>
#include <deadbeef/gtkui_api.h>

DB_functions_t *deadbeef;

static ddb_gtkui_widget_t* w_lyricbar_create (void);

GtkWidget* construct_lyricbar();

int message_handler(struct ddb_gtkui_widget_s *, uint32_t id, uintptr_t ctx, uint32_t, uint32_t);

