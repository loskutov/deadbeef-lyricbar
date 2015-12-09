#include <gtkmm/main.h>
#include <gtkmm/widget.h>
#include <gtkmm/textbuffer.h>
#include <gtkmm/textview.h>
#include <gtkmm/scrolledwindow.h>
#include <string>
#include <iostream> // debugging only
#include <memory>
#include <functional>

#include "ui.h"
#include "utils.h"

using namespace std;
using namespace Gtk;
using namespace Glib;

static unique_ptr<TextView> lyricView;
static unique_ptr<ScrolledWindow> lyricbar;
static RefPtr<TextBuffer> refBuffer;
static RefPtr<TextTag> tagItalic, tagBold, tagLarge;
static vector<RefPtr<TextTag>> tagsTitle;

void set_lyrics(DB_playItem_t * track, const ustring & lyrics) {
    signal_idle().connect([track, lyrics]() -> bool {
        ustring artist, title;
        {
            pl_lock_guard guard;

            if (!is_playing(track))
                return false;
            artist = deadbeef->pl_find_meta(track, "artist");
            title  = deadbeef->pl_find_meta(track, "title");
        }
        refBuffer->erase(refBuffer->begin(), refBuffer->end());
        refBuffer->insert_with_tags(refBuffer->begin(), title, tagsTitle);
        refBuffer->insert_with_tag(refBuffer->end(), ustring("\n") + artist + "\n\n", tagItalic);

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
            refBuffer->insert_with_tags(refBuffer->end(), lyrics.substr(prev_mark, min(bold_mark, italic_mark) - prev_mark), tags);

            if (bold_mark == ustring::npos) {
                prev_mark = italic_mark + 2;
                italic = !italic;
            } else {
                prev_mark = bold_mark + 3;
                bold = !bold;
            }
        }
        refBuffer->insert(refBuffer->end(), lyrics.substr(prev_mark)); // in case if no formatting found

        return false;
    });
}

extern "C"
GtkWidget* construct_lyricbar() {
    Gtk::Main::init_gtkmm_internals();
    refBuffer = TextBuffer::create();

    tagItalic = refBuffer->create_tag();
    tagItalic->property_style() = Pango::STYLE_ITALIC;

    tagBold = refBuffer->create_tag();
    tagBold->property_weight() = Pango::WEIGHT_BOLD;

    tagLarge = refBuffer->create_tag();
    tagLarge->property_scale() = Pango::SCALE_LARGE;

    tagsTitle = {tagLarge, tagBold};

    lyricView = make_unique<TextView>(refBuffer);
    lyricView->set_editable(false);
    lyricView->set_can_focus(false);
    lyricView->set_justification(JUSTIFY_CENTER);
    lyricView->set_wrap_mode(WRAP_WORD_CHAR);
    lyricView->show();
    lyricbar = make_unique<ScrolledWindow>();
    lyricbar->add(*lyricView);
    lyricbar->set_policy(POLICY_AUTOMATIC, POLICY_AUTOMATIC);
    return GTK_WIDGET(lyricbar->gobj());
}

extern "C"
int message_handler(struct ddb_gtkui_widget_s*, uint32_t id, uintptr_t ctx, uint32_t, uint32_t) {
    auto event = reinterpret_cast<ddb_event_track_t *>(ctx);
    switch (id) {
        case DB_EV_SONGSTARTED:
            cerr << "SONGSTARTED" << endl;
        //case DB_EV_TRACKINFOCHANGED:
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
    lyricbar.reset();
    lyricView.reset();
    tagsTitle.clear();
    tagLarge.reset();
    tagBold.reset();
    tagItalic.reset();
    refBuffer.reset();
}

