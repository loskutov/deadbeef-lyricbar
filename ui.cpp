#include <vector>
#include <memory>

#include <glibmm/main.h>
#include <gtkmm/main.h>
#include <gtkmm/widget.h>
#include <gtkmm/textbuffer.h>
#include <gtkmm/textview.h>
#include <gtkmm/scrolledwindow.h>

#include "ui.h"
#include "debug.h"
#include "utils.h"

using namespace std;
using namespace Gtk;
using namespace Glib;

// TODO: eliminate all the global objects, as their initialization is not well defined
static TextView *lyricView;
static ScrolledWindow *lyricbar;
static RefPtr<TextBuffer> refBuffer;
static RefPtr<TextTag> tagItalic, tagBold, tagLarge, tagCenter;
static vector<RefPtr<TextTag>> tagsTitle, tagsArtist;

void set_lyrics(DB_playItem_t *track, ustring lyrics) {
    signal_idle().connect_once([track, lyrics = move(lyrics)]() -> void {
        ustring artist, title;
        {
            pl_lock_guard guard;

            if (!is_playing(track))
                return;
            artist = deadbeef->pl_find_meta(track, "artist");
            title  = deadbeef->pl_find_meta(track, "title");
        }
        refBuffer->erase(refBuffer->begin(), refBuffer->end());
        refBuffer->insert_with_tags(refBuffer->begin(), title, tagsTitle);
        refBuffer->insert_with_tags(refBuffer->end(), ustring("\n") + artist + "\n\n", tagsArtist);

        bool italic = false;
        bool bold = false;
        size_t prev_mark = 0;
        vector<RefPtr<TextTag>> tags;
        while (prev_mark != ustring::npos) {
            size_t italic_mark = lyrics.find("''", prev_mark);
            if (italic_mark == ustring::npos)
                break;
            size_t bold_mark = ustring::npos;
            if (italic_mark < lyrics.size() - 2 && lyrics[italic_mark + 2] == '\'')
                bold_mark = italic_mark;

            tags.clear();
            if (italic) tags.push_back(tagItalic);
            if (bold)   tags.push_back(tagBold);
            refBuffer->insert_with_tags(refBuffer->end(),
                    lyrics.substr(prev_mark, min(bold_mark, italic_mark) - prev_mark), tags);

            if (bold_mark == ustring::npos) {
                prev_mark = italic_mark + 2;
                italic = !italic;
            } else {
                prev_mark = bold_mark + 3;
                bold = !bold;
            }
        }
        refBuffer->insert(refBuffer->end(), lyrics.substr(prev_mark)); // in case if no formatting found
        last = track;
    });
}

Justification get_justification() {
    int align = deadbeef->conf_get_int("lyricbar.lyrics.alignment", 0);
    switch (align) {
        case 0:
            return JUSTIFY_LEFT;
        case 2:
            return JUSTIFY_RIGHT;
        default:
            return JUSTIFY_CENTER;
    }
}

extern "C"
GtkWidget *construct_lyricbar() {
    Gtk::Main::init_gtkmm_internals();
    refBuffer = TextBuffer::create();

    tagItalic = refBuffer->create_tag();
    tagItalic->property_style() = Pango::STYLE_ITALIC;

    tagBold = refBuffer->create_tag();
    tagBold->property_weight() = Pango::WEIGHT_BOLD;

    tagLarge = refBuffer->create_tag();
    tagLarge->property_scale() = Pango::SCALE_LARGE;

    tagCenter = refBuffer->create_tag();
    tagCenter->property_justification() = JUSTIFY_CENTER;

    tagsTitle = {tagLarge, tagBold, tagCenter};
    tagsArtist = {tagItalic, tagCenter};

    lyricView = new TextView(refBuffer);
    lyricView->set_editable(false);
    lyricView->set_can_focus(false);
    lyricView->set_justification(get_justification());
    lyricView->set_wrap_mode(WRAP_WORD_CHAR);
    lyricView->show();

    lyricbar = new ScrolledWindow();
    lyricbar->add(*lyricView);
    lyricbar->set_policy(POLICY_AUTOMATIC, POLICY_AUTOMATIC);

    return GTK_WIDGET(lyricbar->gobj());
}

extern "C"
int message_handler(struct ddb_gtkui_widget_s*, uint32_t id, uintptr_t ctx, uint32_t, uint32_t) {
    auto event = reinterpret_cast<ddb_event_track_t *>(ctx);
    switch (id) {
        case DB_EV_CONFIGCHANGED:
            debug_out << "CONFIG CHANGED\n";
            signal_idle().connect_once([](){ lyricView->set_justification(get_justification()); });
            break;
        case DB_EV_SONGSTARTED:
            debug_out << "SONG STARTED\n";
        case DB_EV_TRACKINFOCHANGED:
            if (!event->track || deadbeef->pl_get_item_duration(event->track) <= 0)
                return 0;
            auto tid = deadbeef->thread_start(update_lyrics, event->track);
            deadbeef->thread_detach(tid);
            break;
    }

    return 0;
}

extern "C"
void lyricbar_destroy() {
    delete lyricbar;
    delete lyricView;
    tagsArtist.clear();
    tagsTitle.clear();
    tagLarge.reset();
    tagBold.reset();
    tagItalic.reset();
    refBuffer.reset();
}

