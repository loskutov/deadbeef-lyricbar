#include <gtkmm/widget.h>
#include <gtkmm/textbuffer.h>
#include <gtkmm/textview.h>
#include <gtkmm/scrolledwindow.h>
#include <string>
#include <iostream> // debugging only
#include <memory>
#include <functional>
#include <algorithm>
#include <curl/curl.h> // curl_easy_escape
#include "utils.h"

extern DB_functions_t *deadbeef;

using namespace std;
using namespace Gtk;
using namespace Glib;

static TextView lyricView;
static ScrolledWindow lyricbar;
static RefPtr<TextBuffer> bufferPtr;

void set_lyrics(DB_playItem_t * track, ustring && lyrics) {
    signal_idle().connect([track, ptr = make_shared<ustring>(lyrics)]() {
        if (!now_playing(track)) {
            return false;
        }
        bufferPtr->set_text(*ptr);//deadbeef->pl_find_meta(track, "artist"));
        //auto tag = bufferPtr->create_tag("b");
        //tag->set_property_value("weight", "bold");
        //bufferPtr->applyTag(
        //bufferPtr->set_text(*ptr);
        return false;
    });
}

void update_lyrics(void * tr) {
    DB_playItem_t * track = (DB_playItem_t*)tr;
    set_lyrics(track, "Loading...");
    auto title = curl_easy_escape(nullptr, deadbeef->pl_find_meta(track, "title"), 0);
    auto artist = curl_easy_escape(nullptr, deadbeef->pl_find_meta(track, "artist"), 0);

    string url;

    try {
        xmlpp::TextReader reader(ustring("http://lyrics.wikia.com/api.php?action=lyrics&fmt=xml&artist=")
                + artist + "&song=" + title);
        curl_free(title);
        curl_free(artist);

        while (reader.read()) {
            if (reader.get_node_type() == xmlpp::TextReader::xmlNodeType::Element
                    && reader.get_name() == "lyrics") {
                reader.read();
                // got the cropped version of lyrics — display it before the complete one is got
                set_lyrics(track, reader.get_value());
                if (reader.get_value() == "Not found")
                    return;
            } else if (reader.get_name() == "url") {
                reader.read();
                url = reader.get_value();
                break;
            }
        }
    } catch (const exception & e) {
        set_lyrics(track, "An error occurred :C");
    }


    url.replace(0, 24, "http://lyrics.wikia.com/api.php?action=query&prop=revisions&rvprop=content&format=xml&titles=");
    cerr << "url = " << url << endl;
    string lyrics;
    try {
        xmlpp::TextReader reader(url);
        while (reader.read()) {
            if (reader.get_name() == "rev") {
                reader.read();
                lyrics = reader.get_value();
                break;
            }
        }
    } catch (const exception & e) {
        cerr << "Much exception: " << e.what() << endl;
    }
    auto pred = [](char c) { return isspace(c); };
    auto fr = find_if_not(lyrics.begin() + lyrics.find('>') + 1, lyrics.end(), pred);
    auto ba = find_if_not(lyrics.rbegin() + lyrics.size() - lyrics.rfind('<'), lyrics.rend(), pred).base();
    set_lyrics(track, ustring(fr, ba));

}

extern "C"
GtkWidget* construct_lyricbar() {
    cerr << "before buffer creation" << endl;
    bufferPtr = Gtk::TextBuffer::create();
    cerr << "after buffer creation" << endl;
    bufferPtr->set_text("KUKAREEEEK!!!!");
    lyricView.set_buffer(bufferPtr);
    lyricView.set_editable(false);
    lyricView.set_can_focus(false);
    lyricView.set_justification(JUSTIFY_CENTER);
    lyricView.set_wrap_mode(WRAP_WORD_CHAR);
    lyricView.show();
    lyricbar.add(lyricView);
    lyricbar.set_policy(POLICY_AUTOMATIC, POLICY_AUTOMATIC);
    return GTK_WIDGET(lyricbar.gobj());
}

extern "C"
int message_handler(struct ddb_gtkui_widget_s*, uint32_t id, uintptr_t ctx, uint32_t, uint32_t) {
    auto event = (ddb_event_track_t *)ctx;
    switch (id) {
        case DB_EV_SONGSTARTED:
            cerr << "SONGSTARTED" << endl;
        //case DB_EV_TRACKINFOCHANGED:
            if (!event->track || deadbeef->pl_get_item_duration(event->track) <= 0)
                return 0;
            printf("lol");
            auto tid = deadbeef->thread_start(update_lyrics, event->track);
            deadbeef->thread_detach(tid);
            break;
    }

    return 0;
}

//void update_lyricbar(const string &lyr_txt, DB_playItem_t *track) {
//}
