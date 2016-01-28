#include <algorithm>
#include <cctype> // ::isspace
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <sys/stat.h>
#include <curl/curl.h> // curl_easy_escape

#include "debug.h"
#include "ui.h"
#include "utils.h"
#include "gettext.h"

using namespace std;
using namespace Glib;

// a simple RAII wrapper around curl_easy_escape
struct escaped_string {
    escaped_string(const char * s) : s(curl_easy_escape(nullptr, s, 0)) {
        if (!s)
            throw runtime_error("curl_easy_escape returned NULL");
    }

    ~escaped_string() {
        curl_free(s);
    }

    operator const char*() const {
        return s;
    }
private:
    char * s;
};


const DB_playItem_t * last;

static const ustring LW_FMT = "http://lyrics.wikia.com/api.php?action=lyrics&fmt=xml&artist=%1&song=%2";

static experimental::optional<string>(*const observers[])(DB_playItem_t *) = {&get_lyrics_from_lyricwiki};

inline string cached_filename(string artist, string title) {
    static const char * home_cache = getenv("XDG_CACHE_HOME");
    static string lyrics_dir = (home_cache ? string(home_cache) : string(getenv("HOME")) + "/.cache")
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
 * @note         Have no idea about the encodings, so a bug possible here
 */
experimental::optional<string> load_cached_lyrics(const string & artist, const string & title) {
    string filename = cached_filename(artist, title);
    debug_out << "filename = '" << filename << "'\n";
    ifstream t(filename);
    if (!t) {
        debug_out << "file '" << filename << "' does not exist :(\n";
        return {};
    }
    stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
}

bool save_cached_lyrics(const string & artist, const string & title, const string & lyrics) {
    string filename = cached_filename(artist, title);
    ofstream t(filename);
    if (!t) {
        cerr << "lyricbar: could not open file for writing: " << filename << endl;
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

experimental::optional<string> get_lyrics_from_lyricwiki(DB_playItem_t * track) {
    const char * artist;
    const char * title;
    {
        pl_lock_guard guard;
        artist = deadbeef->pl_find_meta(track, "artist");
        title  = deadbeef->pl_find_meta(track, "title");
    }

    ustring api_url;
    try {
        escaped_string artist_esc {artist};
        escaped_string title_esc {title};

        api_url = ustring::compose(LW_FMT, artist_esc, title_esc);
    } catch (const exception & e) {
        cerr << "lyricbar: couldn't format API URL, what(): " << e.what() << endl;
        return {};
    }

    string url;
    try {
        xmlpp::TextReader reader(api_url);

        while (reader.read()) {
            if (reader.get_node_type() == xmlpp::TextReader::xmlNodeType::Element
                    && reader.get_name() == "lyrics") {
                reader.read();
                // got the cropped version of lyrics — display it before the complete one is got
                if (reader.get_value() == "Not found")
                    return {};
                else
                    set_lyrics(track, reader.get_value());
            } else if (reader.get_name() == "url") {
                reader.read();
                url = reader.get_value();
                break;
            }
        }
    } catch (const exception & e) {
        cerr << "lyricbar: couldn't parse XML, what(): " << e.what() << endl;
        return {};
    }

    url.replace(0, 24, "http://lyrics.wikia.com/api.php?action=query&prop=revisions&rvprop=content&format=xml&titles=");

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
        cerr << "lyricbar: couldn't parse XML, what(): " << e.what() << endl;
        return {};
    }
    auto front = find_if_not(lyrics.begin() + lyrics.find('>') + 1, lyrics.end(), ::isspace);
    auto back = find_if_not(lyrics.rbegin() + lyrics.size() - lyrics.rfind('<'), lyrics.rend(), ::isspace).base();
    return string(front, back);
}

void update_lyrics(void * tr) {
    DB_playItem_t * track = static_cast<DB_playItem_t*>(tr);
    if (track == last)
        return;

    set_lyrics(track, _("Loading..."));
    const char * artist;
    const char * title;
    {
        pl_lock_guard guard;
        artist = deadbeef->pl_find_meta(track, "artist");
        title  = deadbeef->pl_find_meta(track, "title");
    }

    if (auto lyrics = load_cached_lyrics(artist, title)) {
        set_lyrics(track, *lyrics);
        return;
    }

    for (auto f : observers) {
        if (auto lyrics = f(track)) {
            set_lyrics(track, *lyrics);
            save_cached_lyrics(artist, title, *lyrics);
            return;
        }
    }
    set_lyrics(track, _("Lyrics not found"));
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

int remove_from_cache_action(DB_plugin_action_t *, int ctx) {
    DB_playItem_t * current = nullptr;
    if (ctx == DDB_ACTION_CTX_SELECTION) {
        pl_lock_guard guard;

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
    return 0;
}
