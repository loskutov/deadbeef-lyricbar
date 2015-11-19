#include <curl/curl.h> // curl_easy_escape
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>

#include "utils.h"
#include "ui.h"

using namespace std;
using namespace Glib;

extern DB_functions_t *deadbeef;

static const ustring LW_FMT = "http://lyrics.wikia.com/api.php?action=lyrics&fmt=xml&artist=%1&song=%2";

bool load_cached_lyrics(const char * artist, const char * title, string & ans) {
    string filename = string("/home/ignat/.cache/deadbeef/lyrics/") + artist + '-' + title;
    ifstream t(filename);
    if (!t) {
        cerr << "file '" << filename << "' does not exist :(" << endl;
        return false;
    }
    stringstream buffer;
    buffer << t.rdbuf();
    ans = buffer.str();
    return true;
}

/// Checks whether the given track is playing now
bool now_playing(DB_playItem_t *track) {
    DB_playItem_t *pl_track = deadbeef->streamer_get_playing_track();
    if (!pl_track)
        return false;

    deadbeef->pl_item_unref(pl_track);
    return pl_track == track;
}

void update_lyrics(void * tr) {
    DB_playItem_t * track = static_cast<DB_playItem_t*>(tr);
    set_lyrics(track, "Loading...");

    auto artist = deadbeef->pl_find_meta(track, "artist");
    auto title = deadbeef->pl_find_meta(track, "title");

    string lyrics;
    if (load_cached_lyrics(artist, title, lyrics)) {
        set_lyrics(track, lyrics);
        return;
    }
    auto artist_esc = curl_easy_escape(nullptr, artist, 0);
    auto title_esc = curl_easy_escape(nullptr, title, 0);
    string url;

    try {
        xmlpp::TextReader reader(ustring::compose(LW_FMT, artist_esc, title_esc));
        curl_free(title_esc);
        curl_free(artist_esc);

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
        cerr << "Exception caught while parsing XML: " << e.what() << endl;
        set_lyrics(track, "An error occurred :C");
        return;
    }


    url.replace(0, 24, "http://lyrics.wikia.com/api.php?action=query&prop=revisions&rvprop=content&format=xml&titles=");
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
        cerr << "Exception caught while parsing XML: " << e.what() << endl;
    }
    auto pred = [](char c) { return isspace(c); };
    auto front = find_if_not(lyrics.begin() + lyrics.find('>') + 1, lyrics.end(), pred);
    auto back = find_if_not(lyrics.rbegin() + lyrics.size() - lyrics.rfind('<'), lyrics.rend(), pred).base();
    set_lyrics(track, ustring(front, back));
}

/// Creates the directory tree
int mkpath(string name, mode_t mode) {
    size_t prev = 0, pos;
    string dir;
    int mdret;

    if (name.back() != '/')
        name += '/';

    while ((pos = name.find_first_of('/', prev)) != string::npos){
        dir = name.substr(0, pos++);
        prev = pos;
        if (dir.size() == 0)
            continue; // if leading / first time is 0 length
        if ((mdret = mkdir(dir.c_str(), mode)) && errno != EEXIST)
            return mdret;
    }
    return mdret;
}

