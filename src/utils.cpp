#include "utils.h"

#include <sys/stat.h>

#include <algorithm>
#include <cctype> // ::isspace
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <glibmm/fileutils.h>
#include <glibmm/uriutils.h>

#include "debug.h"
#include "ui.h"
#include "gettext.h"

using namespace std;
using namespace Glib;

const DB_playItem_t *last;

static const ustring LW_FMT = "http://lyrics.wikia.com/api.php?action=lyrics&fmt=xml&artist=%1&song=%2";
static const char *home_cache = getenv("XDG_CACHE_HOME");
static const string lyrics_dir = (home_cache ? string(home_cache) : string(getenv("HOME")) + "/.cache")
                               + "/deadbeef/lyrics/";

static experimental::optional<ustring>(*const observers[])(DB_playItem_t *) = {&observe_lyrics_from_lyricwiki};

inline string cached_filename(string artist, string title) {
    replace(artist.begin(), artist.end(), '/', '_');
    replace(title.begin(), title.end(), '/', '_');

    return lyrics_dir + artist + '-' + title;
}

extern "C"
bool is_cached(const char *artist, const char *title) {
    return artist && title && access(cached_filename(artist, title).c_str(), 0) == 0;
}

extern "C"
void ensure_lyrics_path_exists() {
    mkpath(lyrics_dir, 0755);
}

/**
 * Loads the cached lyrics
 * @param artist The artist name
 * @param title  The song title
 * @note         Have no idea about the encodings, so a bug possible here
 */
experimental::optional<ustring> load_cached_lyrics(const char *artist, const char *title) {
    string filename = cached_filename(artist, title);
    debug_out << "filename = '" << filename << "'\n";
    try {
        return {file_get_contents(filename)};
    } catch (const FileError& error) {
        debug_out << error.what();
        return {};
    }
}

bool save_cached_lyrics(const string &artist, const string &title, const string &lyrics) {
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

static
experimental::optional<ustring> get_lyrics_from_id3v2(DB_playItem_t *track) {
    const char *path;
    {
        pl_lock_guard guard;
        path = deadbeef->pl_find_meta(track, ":URI");
    }

    DB_FILE *fp = deadbeef->fopen(path);
    if (!fp) {
        cerr << "lyricbar: tried to get lyrics from tag but couldn't fopen the file" << endl;
        return {};
    }

    id3v2_tag id3;
    int res = deadbeef->junk_id3v2_read_full(track, &id3.tag, fp);

    deadbeef->fclose(fp);

    if (res != 0) {
        debug_out << "junk_id3v2_read_full returned " << res << endl;
        return {};
    }

    for (auto frame = id3.tag.frames; frame; frame = frame->next) {
        if (!strcmp(frame->id, "USLT") && frame->size > 5)
            return ustring{reinterpret_cast<const char*>(frame->data + 5)};
    }

    return {};
}

static
experimental::optional<ustring> get_lyrics_from_metadata(DB_playItem_t *track) {
    pl_lock_guard guard;
    const char *lyrics = deadbeef->pl_find_meta(track, "lyrics");
    if (lyrics)
        return ustring{lyrics};
    else return {};
}

experimental::optional<ustring> get_lyrics_from_tag(DB_playItem_t *track) {
    if (auto ans = get_lyrics_from_metadata(track))
        return ans;
    else return get_lyrics_from_id3v2(track);
}

experimental::optional<ustring> observe_lyrics_from_lyricwiki(DB_playItem_t *track) {
    const char *artist;
    const char *title;
    {
        pl_lock_guard guard;
        artist = deadbeef->pl_find_meta(track, "artist");
        title  = deadbeef->pl_find_meta(track, "title");
    }

    ustring api_url = ustring::compose(LW_FMT, uri_escape_string(artist, {}, false)
                                             , uri_escape_string(title, {}, false));

    string url;
    try {
        xmlpp::TextReader reader(api_url);

        while (reader.read()) {
            if (reader.get_node_type() == xmlpp::TextReader::NodeType::Element
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
    } catch (const exception &e) {
        cerr << "lyricbar: couldn't parse XML (URI is '" << api_url << "'), what(): " << e.what() << endl;
        return {};
    }

    url.replace(0, 24, "http://lyrics.wikia.com/api.php?action=query&prop=revisions&rvprop=content&format=xml&titles=");

    ustring raw_lyrics;
    try {
        xmlpp::TextReader reader(url);
        while (reader.read()) {
            if (reader.get_name() == "rev") {
                reader.read();
                raw_lyrics = reader.get_value();
                break;
            }
        }
    } catch (const exception &e) {
        cerr << "lyricbar: couldn't parse XML, what(): " << e.what() << endl;
        return {};
    }

    raw_lyrics = "<root>" + raw_lyrics + "</root>";  // a somewhat dirty hack, but that's life

    ustring lyrics;
    try {
        xmlpp::TextReader reader(reinterpret_cast<const uint8_t*>(raw_lyrics.data()), raw_lyrics.bytes());
        while (reader.read()) {
            if (reader.get_name() == "lyrics") {
                reader.read();
                lyrics = reader.get_value();
                break;
            }
        }
    } catch (const exception &e) {
        cerr << "lyricbar: couldn't parse raw lyrics, what(): " << e.what() << endl;
        return {};
    }
    auto after_last_nonspace = find_if_not(lyrics.rbegin(), lyrics.rend(), ::isspace).base();
    lyrics.erase(after_last_nonspace, lyrics.end());
    auto first_nonspace = find_if_not(lyrics.begin(), lyrics.end(), ::isspace);
    lyrics.erase(lyrics.begin(), first_nonspace);
    return lyrics;
}

void update_lyrics(void *tr) {
    DB_playItem_t *track = static_cast<DB_playItem_t*>(tr);
    if (track == last)
        return;

    if (auto lyrics = get_lyrics_from_tag(track)) {
        set_lyrics(track, *lyrics);
        return;
    }

    set_lyrics(track, _("Loading..."));

    const char *artist;
    const char *title;
    {
        pl_lock_guard guard;
        artist = deadbeef->pl_find_meta(track, "artist");
        title  = deadbeef->pl_find_meta(track, "title");
    }

    if (artist && title) {
        if (auto lyrics = load_cached_lyrics(artist, title)) {
            set_lyrics(track, *lyrics);
            return;
        }

        // No lyrics in the tag or cache; try to get some and cache if succeeded
        for (auto f : observers) {
            if (auto lyrics = f(track)) {
                set_lyrics(track, *lyrics);
                save_cached_lyrics(artist, title, *lyrics);
                return;
            }
        }
    }
    set_lyrics(track, _("Lyrics not found"));
}

/**
 * Creates the directory tree.
 * @param name the directory path, including trailing slash
 * @return 0 on success; errno after mkdir call if something went wrong
 */
int mkpath(const string &name, mode_t mode) {
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
    if (ctx == DDB_ACTION_CTX_SELECTION) {
        pl_lock_guard guard;

        ddb_playlist_t *playlist = deadbeef->plt_get_curr();
        if (playlist) {
            DB_playItem_t *current = deadbeef->plt_get_first(playlist, PL_MAIN);
            while (current) {
                if (deadbeef->pl_is_selected (current)) {
                    const char *artist = deadbeef->pl_find_meta(current, "artist");
                    const char *title  = deadbeef->pl_find_meta(current, "title");
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
