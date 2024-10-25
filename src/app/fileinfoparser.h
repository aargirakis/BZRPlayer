#ifndef FILEINFOPARSER_H
#define FILEINFOPARSER_H

#include "playlistitem.h"
#include <QTableWidget>

class FileInfoParser
{
public:
    FileInfoParser();
    void updateFileInfo(QTableWidget*, PlaylistItem*);

private:
    static const string ID3V1_GENRES[];
};

#endif // FILEINFOPARSER_H
