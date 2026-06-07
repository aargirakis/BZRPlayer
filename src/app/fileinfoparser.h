#ifndef FILEINFOPARSER_H
#define FILEINFOPARSER_H

#include <QTableWidget>
#include "playlistitem.h"

class FileInfoParser {
public:
    FileInfoParser();

    static void updateFileInfo(QTableWidget *, const PlaylistItem *);

private:
    static void addInfo(QTableWidget *tableInfo, int *row, const QString &label, const QString &value);

    static void addMultilineInfo(QTableWidget *tableInfo, int *row, const QString &label, const string &value);

    static void addSubsongInfo(QTableWidget *tableInfo, int *row);

    static void showFmodSupportedTagsIfAny(QTableWidget *tableInfo, const PlaylistItem *playlistItem, int *row);
};

#endif // FILEINFOPARSER_H
