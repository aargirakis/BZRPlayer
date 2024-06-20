#ifndef ALBUMGRID_H
#define ALBUMGRID_H
#include <QListWidget>

class Album;

class AlbumGrid : public QListWidget
{
    Q_OBJECT
public:
    AlbumGrid(QWidget *parent= nullptr);
    void AddAlbum(Album*);
    void Filter(QString);
private:
    QHash<QString,Album*> albumsMap;
    QList<QListWidgetItem*> itemsList;
};

#endif // ALBUMGRID_H
