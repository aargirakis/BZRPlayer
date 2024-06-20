#ifndef PLAYLISTITEM_H
#define PLAYLISTITEM_H
#include <QString>
#include "info.h"
class PlaylistItem
{
public:
    PlaylistItem();
    PlaylistItem(Info*,QString,int);
    QString fullPath;
    QString artist;
    QString filename;
    QString fileFormat;
    QString path;
    QString title;
    Info* info;
    int subsong;
    int subsongs;
    unsigned int startTime;
    unsigned int startSubsong;
    signed int startSubsongPlayList;
    signed int length;
};

#endif // PLAYLISTITEM_H
