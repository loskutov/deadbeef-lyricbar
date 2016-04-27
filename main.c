#include <string.h>
#include <stdlib.h>

#include "main.h"
#include "ui.h"
#include "utils.h"
#include "gettext.h"

ddb_gtkui_t *gtkui_plugin;
DB_functions_t *deadbeef;
static DB_misc_t plugin;

static const char settings_dlg[] = "property \"Lyrics alignment type\" select[3] lyricbar.lyrics.alignment 0 left center right;";

static int lyricbar_disconnect() {
    if (gtkui_plugin) {
        gtkui_plugin->w_unreg_widget(plugin.plugin.id);
    }
    return 0;
}

DB_plugin_action_t remove_action = {
    .name = "remove_lyrics",
    .flags = DB_ACTION_MULTIPLE_TRACKS | DB_ACTION_ADD_MENU,
    .callback2 = remove_from_cache_action,
    .next = NULL,
    .title = "Remove Lyrics From Cache"
};

static DB_plugin_action_t *
lyricbar_get_actions() {
    deadbeef->pl_lock();
    remove_action.flags |= DB_ACTION_DISABLED;
    DB_playItem_t *current = deadbeef->pl_get_first(PL_MAIN);
    while (current) {
        if (deadbeef->pl_is_selected(current) && is_cached(
                    deadbeef->pl_find_meta(current, "artist"),
                    deadbeef->pl_find_meta(current, "title"))) {
            remove_action.flags &= ~DB_ACTION_DISABLED;
            deadbeef->pl_item_unref(current);
            break;
        }
        DB_playItem_t *next = deadbeef->pl_get_next(current, PL_MAIN);
        deadbeef->pl_item_unref(current);
        current = next;
    }
    deadbeef->pl_unlock();
    return &remove_action;
}

static ddb_gtkui_widget_t*
w_lyricbar_create(void) {
    ddb_gtkui_widget_t *widget = malloc(sizeof(ddb_gtkui_widget_t));
    memset(widget, 0, sizeof(ddb_gtkui_widget_t));

    widget->widget  = construct_lyricbar();
    widget->destroy = lyricbar_destroy;
    widget->message = message_handler;

    gtkui_plugin->w_override_signals(widget->widget, widget);
    return widget;
}

static int lyricbar_connect() {
    gtkui_plugin = (ddb_gtkui_t *)deadbeef->plug_get_for_id(DDB_GTKUI_PLUGIN_ID);
    if (!gtkui_plugin) {
        fprintf(stderr, "%s: can't find gtkui plugin\n", plugin.plugin.id);
        return -1;
    }
    gtkui_plugin->w_reg_widget("Lyricbar", 0, w_lyricbar_create, "lyricbar", NULL);
    return 0;
}

#if GTK_MAJOR_VERSION == 2
DB_plugin_t *ddb_lyricbar_gtk2_load(DB_functions_t *ddb) {
#else
DB_plugin_t *ddb_lyricbar_gtk3_load(DB_functions_t *ddb) {
#endif
    deadbeef = ddb;
    setlocale(LC_ALL, "");
    bindtextdomain("deadbeef-lyricbar", "/usr/share/locale");
    textdomain("deadbeef-lyricbar");
    remove_action.title = _(remove_action.title);
    return DB_PLUGIN(&plugin);
}


static DB_misc_t plugin = {
    .plugin.api_vmajor = 1,
    .plugin.api_vminor = 5,
    .plugin.version_major = 1,
    .plugin.version_minor = 0,
    .plugin.type = DB_PLUGIN_MISC,
    .plugin.name = "Lyricbar",
#if GTK_MAJOR_VERSION == 2
    .plugin.id = "lyricbar-gtk2",
#else
    .plugin.id = "lyricbar-gtk3",
#endif
    .plugin.descr = "Lyricbar plugin for DeadBeeF audio player.\nFetches and shows song’s lyrics.\n",
    .plugin.copyright = "Copyright (C) 2015 Ignat Loskutov <ignat.loskutov@gmail.com>\n",
    .plugin.website = "https://github.com/loskutov/deadbeef-lyricbar",
    .plugin.connect = lyricbar_connect,
    .plugin.disconnect = lyricbar_disconnect,
    .plugin.configdialog = settings_dlg,
    .plugin.get_actions = lyricbar_get_actions
};

