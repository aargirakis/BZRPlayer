#ifndef MYMODEL_H
#define MYMODEL_H

#include <QUuid>
#include "mainwindow.h"

struct Item {
    QString title;
    QString fileFormat;
    QString lengthStr;
    QString subsong;
    QString fullPath;
    int lengthInt;
    bool isPlayable;
    bool isPlaying;
    QString artist;
    QString filename;
    QString path;
    Info *info;
    int subsongs;
    unsigned int startTime;
    unsigned int startSubsong;
    signed int startSubsongPlayList;
    QUuid uuid = QUuid::createUuid();

    bool operator==(const Item &other) const {
        return uuid == other.uuid;
    }
};

class PlaylistModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit PlaylistModel(QObject *parent = nullptr);

    enum Section { Title, FileFormat, LengthStr, Subsong, FullPath, LengthInt, IsPlayable, IsPlaying, Artist };

    Qt::DropActions supportedDropActions() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;

    bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;

    QMimeData *mimeData(const QModelIndexList &indexes) const override;

    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column, const QModelIndex &parent) override;

    QStringList mimeTypes() const override;

    void setDropTargetRow(int row);

    int findRowByUuid(const QUuid &uuid) const;

private:
    MainWindow *m_root;
    QList<Item> items;
    int m_pendingDropRow = -1;
};

#endif // MYMODEL_H
