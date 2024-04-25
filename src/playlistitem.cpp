#include "playlistitem.h"

PlaylistItem::PlaylistItem(Info* i, QString fullpath,int subsong)
{
    info = i;
    this->fullPath = fullpath;
    this->subsong = subsong;
    this->length=-1;
    this->startSubsongPlayList=-1;
}
PlaylistItem::PlaylistItem()
{
    this->length=-1;
    this->startSubsongPlayList=-1;
}
