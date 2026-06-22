#include <QMimeData>
#include "playlistmodel.h"

PlaylistModel::PlaylistModel(QObject *parent) : QAbstractTableModel(parent) {
    m_root = static_cast<MainWindow *>(parent);
    //currentRowPlaying = 0;
}

int PlaylistModel::rowCount(const QModelIndex & /* parent */) const {
    return items.size();
}

int PlaylistModel::columnCount(const QModelIndex & /* parent */) const {
    return 9;
}

Qt::DropActions PlaylistModel::supportedDropActions() const {
    return Qt::MoveAction;
}

QVariant PlaylistModel::data(const QModelIndex &index, const int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    const auto &contact = items.at(index.row());

    if (index.row() >= items.size() || index.row() < 0)
        return QVariant();

    if (role == Qt::DisplayRole) {
        if (index.column() == Title)
            return contact.title;
        if (index.column() == FileFormat)
            return contact.fileFormat;
        if (index.column() == LengthStr)
            return contact.lengthStr;
        if (index.column() == Subsong)
            return contact.subsong;
        if (index.column() == FullPath)
            return contact.fullPath;
        if (index.column() == LengthInt)
            return contact.lengthInt;
        if (index.column() == IsPlayable)
            return contact.isPlayable;
        if (index.column() == IsPlaying)
            return contact.isPlaying;
        if (index.column() == Artist)
            return contact.artist;
    } else if (role == Qt::ForegroundRole && contact.isPlaying)
    //else if (role == Qt::ForegroundRole && index.row() == currentRowPlaying)
    {
        return QColor(m_root->getColorMain().left(7));
    } else if (role == Qt::ForegroundRole && contact.isPlayable) {
        QColor maintext(m_root->getColorMainText().left(7));
        maintext.setAlpha(128);
        return maintext;
    }

    return QVariant();
}

QVariant PlaylistModel::headerData(const int section, const Qt::Orientation orientation, const int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) return {};

    switch (section) {
        case 0: return "TITLE";
        case 1: return "FORMAT";
        case 2: return "LENGTH";
        case 3: return "SUBSONG";
        case 4: return "FULLPATH";
        case 5: return "LENGTH int";
        case 6: return "PLAYABLE";
        case 7: return "IS PLAYING";
        case 8: return "ARTIST";
        default: return {};
    }
}

Qt::ItemFlags PlaylistModel::flags(const QModelIndex &index) const {
    const Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);

    if (index.isValid())
        return defaultFlags | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;

    return defaultFlags | Qt::ItemIsDropEnabled;
}

bool PlaylistModel::insertRows(const int position, const int rows, const QModelIndex &index) {
    Q_UNUSED(index);
    beginInsertRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row) {
        items.insert(position, {});
    }

    endInsertRows();
    return true;
}

bool PlaylistModel::removeRows(const int position, const int rows, const QModelIndex &index) {
    Q_UNUSED(index);
    beginRemoveRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row) {
        items.removeAt(position);
    }

    endRemoveRows();
    return true;
}

bool PlaylistModel::setData(const QModelIndex &index, const QVariant &value, const int role) {
    //    if(role == Qt::ForegroundRole && index.column() == Title)
    //    {
    //        currentRowPlaying=value.toInt();
    //    }
    const int row = index.row();

    //cout << "setData row " << row << "\n";

    if (role == Qt::EditRole || Qt::DisplayRole) {
        auto contact = items.value(row);

        if (index.column() == Title)
            contact.title = value.toString();
        else if (index.column() == FileFormat)
            contact.fileFormat = value.toString();
        else if (index.column() == LengthStr)
            contact.lengthStr = value.toString();
        else if (index.column() == Subsong) {
            if (value == -1)
                contact.subsong = "";
            else
                contact.subsong = value.toString();
        } else if (index.column() == FullPath)
            contact.fullPath = value.toString();
        else if (index.column() == LengthInt)
            contact.lengthInt = value.toInt();
        else if (index.column() == IsPlayable)
            contact.isPlayable = value.toBool();
        else if (index.column() == IsPlaying)
            contact.isPlaying = value.toBool();
        else if (index.column() == Artist)
            contact.artist = value.toString();
        else
            return false;

        items.replace(row, contact);
        emit dataChanged(index, index);
        return true;
    }

    if (role == Qt::ForegroundRole) {
        return true;
    }
}

QStringList PlaylistModel::mimeTypes() const {
    return {"application/vnd.text.list"}; // use any string, just keep it consistent
}

QMimeData *PlaylistModel::mimeData(const QModelIndexList &indexes) const {
    const auto mimeData = new QMimeData();
    QByteArray encoded;
    QDataStream stream(&encoded, QIODevice::WriteOnly);

    QSet<int> rows;

    for (const QModelIndex &index: indexes)
        rows.insert(index.row());

    QList<int> sortedRows = rows.values();
    std::sort(sortedRows.begin(), sortedRows.end());

    for (const int row: sortedRows)
        stream << row;

    mimeData->setData("application/vnd.text.list", encoded);
    return mimeData;
}

bool PlaylistModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                 int row, int column, const QModelIndex &parent) {
    if (action != Qt::MoveAction || !data->hasFormat("application/vnd.text.list"))
        return false;

    // save a pointer/reference to the item at currentRow
    Item trackedItem;
    int currentRow = m_root->getCurrentRow();

    if (currentRow >= 0) {
        trackedItem = items[currentRow];
    }

    QByteArray encoded = data->data("application/vnd.text.list");
    QDataStream stream(&encoded, QIODevice::ReadOnly);

    QList<int> sourceRows;

    while (!stream.atEnd()) {
        int r;
        stream >> r;
        if (r >= 0 && r < items.size())
            sourceRows.append(r);
    }

    if (sourceRows.isEmpty())
        return false;

    std::sort(sourceRows.begin(), sourceRows.end());

    // use the drop row passed from the view (via setDropTargetRow)
    int destinationRow = m_pendingDropRow != -1 ? m_pendingDropRow : row;
    m_pendingDropRow = -1; // reset after using

    int movedBeforeTarget = 0;

    for (int r: sourceRows) {
        if (r < destinationRow)
            ++movedBeforeTarget;
    }

    destinationRow -= movedBeforeTarget;

    // clamp to valid index range
    destinationRow = std::clamp(destinationRow, 0, static_cast<int>(items.size()));

    // remove the source items from the model
    QList<Item> movedItems;

    for (int i = sourceRows.size() - 1; i >= 0; --i)
        movedItems.prepend(items.takeAt(sourceRows[i]));

    // insert them at the destination
    beginResetModel();

    for (int i = 0; i < movedItems.count(); ++i)
        items.insert(destinationRow + i, movedItems[i]);

    endResetModel();

    // reset shuffle if shuffle is enabled
    if (m_root->isShuffleEnabled()) {
        if (m_root->getSelectedPlaylist() == m_root->getCurrentPlaylist()) {
            m_root->resetShuffle(m_root->getCurrentPlaylist());
        }
    }

    // set new currentRow
    if (currentRow >= 0) {
        int newRow = findRowByUuid(trackedItem.uuid);
        m_root->setCurrentRow(newRow);
    }

    return true;
}

void PlaylistModel::setDropTargetRow(const int row) {
    m_pendingDropRow = row;
}

int PlaylistModel::findRowByUuid(const QUuid &uuid) const {
    for (int i = 0; i < items.size(); ++i) {
        if (items[i].uuid == uuid)
            return i;
    }

    return -1;
}
