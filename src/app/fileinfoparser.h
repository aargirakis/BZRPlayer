#ifndef FILEINFOPARSER_H
#define FILEINFOPARSER_H

#include "playlistitem.h"
#include <QTableWidget>

class FileInfoParser
{
public:
    FileInfoParser();
    static void updateFileInfo(QTableWidget*, PlaylistItem*);

private:
    static constexpr unsigned int id3v1GenresMax = 191;
    static const string ID3V1_GENRES[];

    static void addInfo(QTableWidget *tableInfo, int *row, const QString &label, const QString &value);
    static void addMultilineInfo(QTableWidget *tableInfo, int *row, const QString &label, const string &value);
    static void addLengthInfo(QTableWidget *tableInfo, const PlaylistItem *playlistItem, int *row);
    static void addSubsongInfo(QTableWidget *tableInfo, int *row);
    static void addSidClockSpeed(QTableWidget *tableInfo, int *row);
    static void addSidCompatibility(QTableWidget *tableInfo, int *row);
    static void addAsapClockSpeed(QTableWidget *tableInfo, int *row);
    static void showFmodSupportedTagsIfAny(QTableWidget *tableInfo, const PlaylistItem *playlistItem, int *row);
};

#endif // FILEINFOPARSER_H
