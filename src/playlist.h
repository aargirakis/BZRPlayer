#ifndef PLAYLIST_H
#define PLAYLIST_H

#include "playlistitem.h"
class Playlist
{
public:
    Playlist();
private:
    vector<PlaylistItem*> playListitems;
    QString name;
    QString fullpath;
};

#endif // PLAYLIST_H
