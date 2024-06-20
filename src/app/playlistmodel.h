#ifndef MYMODEL_H
#define MYMODEL_H

#include "playlistitem.h"
#include <QAbstractTableModel>
#include "mainwindow.h"

struct Item
  {
    bool isPlaying;
    bool playable;
    QString fullPath;
    QString filename;
    QString fileFormat;
    QString path;
    QString title;
    Info* info;
    QString subsong;
    QString artist;
    int subsongs;
    unsigned int startTime;
    unsigned int startSubsong;
    signed int startSubsongPlayList;
    QString length;
    int lengthInt;
  };

class PlaylistModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit PlaylistModel(QObject *parent = nullptr);
    Qt::DropActions supportedDropActions() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;
    bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;

private:
    MainWindow* m_root;
    QList<Item> items;
    //int currentRowPlaying;
};

#endif // MYMODEL_H
