#include "main.h"
#include "ui.h"
#include <string.h>
#include <stdlib.h>

ddb_gtkui_t * gtkui_plugin;
DB_functions_t * deadbeef;

static const char settings_dlg[] =
    "property \"Lyrics alignment type\" select[3] lyricbar.lyrics.alignment 0 left center right;"
;

void lyricbar_init(struct ddb_gtkui_widget_s *widget) {
    puts("init called");
}

static ddb_gtkui_widget_t*
w_lyricbar_create (void) {
    ddb_gtkui_widget_t *widget = malloc(sizeof(ddb_gtkui_widget_t));
    memset(widget, 0, sizeof(ddb_gtkui_widget_t));

    widget->widget  = construct_lyricbar();
    widget->init    = lyricbar_init;
    widget->destroy = lyricbar_destroy;
    widget->message = message_handler;

    gtkui_plugin->w_override_signals(widget->widget, widget);
    return widget;
}

static int lyricbar_connect() {
    gtkui_plugin = (ddb_gtkui_t *)deadbeef->plug_get_for_id(DDB_GTKUI_PLUGIN_ID);
    if (!gtkui_plugin) {
        fprintf(stderr, "lyricbar: can’t find gtkui plugin\n");
        return -1;
    }
    gtkui_plugin->w_reg_widget("Lyricbar", 0, w_lyricbar_create, "lyricbar", NULL);
    return 0;
}

static int lyricbar_disconnect() {
    if (gtkui_plugin) {
        gtkui_plugin->w_unreg_widget("lyricbar");
    }
    return 0;
}

static DB_misc_t plugin = {
    .plugin.api_vmajor = 1,
    .plugin.api_vminor = 5,
    .plugin.version_major = 1,
    .plugin.version_minor = 0,
    .plugin.type = DB_PLUGIN_MISC,
    .plugin.name = "Lyricbar",
    .plugin.id = "lyricbar",
    .plugin.descr = "Lyricbar plugin for DeadBeeF audio player.\nFetches and shows song’s lyrics.\n",
    .plugin.copyright = "Copyright (C) 2015 Ignat Loskutov <ignat.loskutov@gmail.com>\n",
    .plugin.website = "https://github.com/loskutov/deadbeef-lyricbar",
    .plugin.connect = lyricbar_connect,
    .plugin.disconnect = lyricbar_disconnect,
    .plugin.configdialog = settings_dlg
};

DB_plugin_t * ddb_lyricbar_load(DB_functions_t *ddb) {
    deadbeef = ddb;
    return DB_PLUGIN(&plugin);
}

