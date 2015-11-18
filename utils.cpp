#include "utils.h"

using namespace std;

extern DB_functions_t *deadbeef;

/// Checks whether the given track is playing now
bool now_playing(DB_playItem_t *track) {
    DB_playItem_t *pl_track = deadbeef->streamer_get_playing_track();
    if (!pl_track)
        return false;

    deadbeef->pl_item_unref(pl_track);
    return pl_track == track;
}


int main() {
    //string s = dl_file("http://tbp.karelia.ru/");
    //cout << s << endl;
}
