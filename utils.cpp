#include <curl/curl.h> // curl_easy_escape
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>

#include "utils.h"
#include "ui.h"
#include "gettext.h"

using namespace std;
using namespace Glib;

extern DB_functions_t * deadbeef;

static const ustring LW_FMT = "http://lyrics.wikia.com/api.php?action=lyrics&fmt=xml&artist=%1&song=%2";

inline string cached_filename(string artist, string title) {
    const char * home_cache = getenv("XDG_CACHE_HOME");
    string lyrics_dir = (home_cache ? string(home_cache) : string(getenv("HOME")) + "/.cache")
        + "/deadbeef/lyrics/";

    replace(artist.begin(), artist.end(), '/', '_');
    replace(title.begin(), title.end(), '/', '_');

    return lyrics_dir + artist + '-' + title;
}

extern "C"
bool is_cached(const char * artist, const char * title) {
    return access(cached_filename(artist, title).c_str(), 0) == 0;
}

/**
 * Loads the cached lyrics
 * @param artist The artist name
 * @param title  The song title
 * @param ans    The string to put the lyrics into
 * @return       Whether succeeded or not
 * @note         Have no idea about the encodings, so a bug possible here
 */
bool load_cached_lyrics(const string & artist, const string & title, string & ans) {
    string filename = cached_filename(artist, title);
    cerr << "filename = '" << filename << "'" << endl;
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

bool save_cached_lyrics(const string & artist, const string & title, const ustring & lyrics) {
    string filename = cached_filename(artist, title);
    ofstream t(filename);
    if (!t) {
        cerr << "lyricbar: could not open file for writing" << endl;
        return false;
    }
    t << lyrics;
    return true;
}

bool is_playing(DB_playItem_t *track) {
    DB_playItem_t *pl_track = deadbeef->streamer_get_playing_track();
    if (!pl_track)
        return false;

    deadbeef->pl_item_unref(pl_track);
    return pl_track == track;
}

void update_lyrics(void * tr) {
    DB_playItem_t * track = static_cast<DB_playItem_t*>(tr);
    set_lyrics(track, _("Loading..."));

    const char * artist;
    const char * title;
    {
        pl_lock_guard guard;
        artist = deadbeef->pl_find_meta(track, "artist");
        title  = deadbeef->pl_find_meta(track, "title");
    }

    string lyrics;
    if (load_cached_lyrics(artist, title, lyrics)) {
        set_lyrics(track, lyrics);
        cerr << "loaded cached" << endl;
        return;
    }
    auto artist_esc = curl_easy_escape(nullptr, artist, 0);
    auto title_esc  = curl_easy_escape(nullptr, title, 0);
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
        cerr << "lyricbar: exception caught while parsing XML: " << e.what() << endl;
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
        cerr << "lyricbar: exception caught while parsing XML: " << e.what() << endl;
    }
    auto pred = [](char c) { return isspace(c); };
    auto front = find_if_not(lyrics.begin() + lyrics.find('>') + 1, lyrics.end(), pred);
    auto back = find_if_not(lyrics.rbegin() + lyrics.size() - lyrics.rfind('<'), lyrics.rend(), pred).base();
    lyrics = string(front, back);
    set_lyrics(track, lyrics);
    save_cached_lyrics(artist, title, lyrics);
}

/**
 * Creates the directory tree.
 * @param name the directory path, including trailing slash
 * @return 0 on success; errno after mkdir call if something went wrong
 */
int mkpath(const string & name, mode_t mode) {
    string dir;
    size_t pos = 0;
    while ((pos = name.find_first_of('/', pos)) != string::npos){
        dir = name.substr(0, pos++);
        if (dir.size() == 0)
            continue; // ignore the leading slash
        if (mkdir(dir.c_str(), mode) && errno != EEXIST)
            return errno;
    }
    return 0;
}

int remove_from_cache_action(DB_plugin_action_t *action, int ctx) {
    DB_playItem_t * current = nullptr;
    pl_lock_guard guard;
    if (ctx == DDB_ACTION_CTX_SELECTION) {
        ddb_playlist_t *playlist = deadbeef->plt_get_curr();
        if (playlist) {
            current = deadbeef->plt_get_first(playlist, PL_MAIN);
            while (current) {
                if (deadbeef->pl_is_selected (current)) {
                    const char * artist = deadbeef->pl_find_meta(current, "artist");
                    const char * title  = deadbeef->pl_find_meta(current, "title");
                    if (is_cached(artist, title))
                        remove(cached_filename(artist, title).c_str());
                }
                DB_playItem_t *next = deadbeef->pl_get_next(current, PL_MAIN);
                deadbeef->pl_item_unref(current);
                current = next;
            }
            deadbeef->plt_unref(playlist);
        }
    }
    if (current)
        deadbeef->pl_item_unref(current);
    return 0;
}
