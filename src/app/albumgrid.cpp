#include "album.h"
#include "albumgrid.h"

AlbumGrid::AlbumGrid(QWidget *parent) : QListWidget(parent)
{
    this->setMinimumHeight(128);
    this->setViewMode(IconMode);
    this->setResizeMode(Adjust);
    this->setWrapping(true);
    this->setAcceptDrops(false);
    this->setDragDropMode(DragOnly);

    this->setSizePolicy(QSizePolicy ::Expanding , QSizePolicy ::Expanding );
    this->setSizeAdjustPolicy(AdjustToContentsOnFirstShow);

    this->setStyleSheet("QWidget{background-color:#000000;}QListWidget::item,QListWidget::item:selected,QListWidget::item:selected:active {background:transparent; color:transparent; }");
    this->setGridSize(QSize(124,150));
    this->setContextMenuPolicy(Qt::ContextMenuPolicy::ActionsContextMenu);

}

void AlbumGrid::AddAlbum(Album* album)
{
    this->albumsMap.insert(album->id,album);

    const auto item = new QListWidgetItem;

    this->itemsList.push_back(item);
    item->setSizeHint(QSize(124,150));
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(album->title);
    this->addItem(item);
    this->setItemWidget(item,album);
}

void AlbumGrid::Filter(const QString &filter)
{

    for( int i = 0; i < this->count(); ++i )
    {
        bool match = false;

        if (const QListWidgetItem *item = this->item(i);
            item->text().contains(filter, Qt::CaseInsensitive)) {
            match = true;
            break;
        }

        this->setRowHidden(i, !match);
    }
}
