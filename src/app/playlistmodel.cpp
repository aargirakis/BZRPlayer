#include "playlistmodel.h"
#include "playlistitem.h"
#include "qbrush.h"
#include "qdebug.h"
#include "qfont.h"

PlaylistModel::PlaylistModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_root = static_cast<MainWindow*>(parent);
    //currentRowPlaying=0;
}



int PlaylistModel::rowCount(const QModelIndex & /*parent*/) const
{
   return items.size();
}

int PlaylistModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 9;
}
Qt::DropActions PlaylistModel::supportedDropActions() const
 {
     return Qt::MoveAction;
 }

QVariant PlaylistModel::data(const QModelIndex &index, int role) const
{


    if (!index.isValid())
    {
        return QVariant();
    }
    const auto &contact = items.at(index.row());
    if (index.row() >= items.size() || index.row() < 0)
        return QVariant();

     if (role == Qt::DisplayRole) {




        if (index.column() == 0)
            return contact.title;
        else if (index.column() == 1)
            return contact.fileFormat;
        else if (index.column() == 2)
            return contact.length;
        else if (index.column() == 3)
            return contact.subsong;
        else if (index.column() == 4)
            return contact.fullPath;
        else if (index.column() == 5)
        {
            return contact.lengthInt;
        }
        else if (index.column() == 6)
        {
            return contact.playable;
        }
        else if (index.column() == 7)
        {
            return contact.isPlaying;
        }
        else if (index.column() == 8)
        {
            return contact.artist;
        }
    }
     else if (role == Qt::ForegroundRole && contact.isPlaying)
     //else if (role == Qt::ForegroundRole && index.row() == currentRowPlaying)
     {
          return QColor(m_root->getColorMain().left(7));
     }
    else if(role == Qt::ForegroundRole && contact.playable)
    {
        QColor maintext(m_root->getColorMainText().left(7));
        maintext.setAlpha(128);
        return maintext;
    }


    return QVariant();
}


QVariant PlaylistModel::headerData(int section, Qt::Orientation orientation, int role) const
{
      if (orientation != Qt::Horizontal || role != Qt::DisplayRole) return {};
      switch (section) {
        case 0: return "TITLE";
        case 1: return "TYPE";
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
Qt::ItemFlags PlaylistModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
bool PlaylistModel::insertRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginInsertRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row)
    {
        items.insert(position, { });
    }
    endInsertRows();
    return true;
}
bool PlaylistModel::removeRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginRemoveRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row)
    {
        items.removeAt(position);
    }

    endRemoveRows();
    return true;
}
bool PlaylistModel::setData(const QModelIndex &index, const QVariant &value, int role)
  {


//    if(role==Qt::ForegroundRole && index.column()==0)
//    {
//        currentRowPlaying=value.toInt();
//    }
    int row = index.row();

    //cout << "setData row " << row << "\n";

    if (role == Qt::EditRole || Qt::DisplayRole)
    {



        auto contact = items.value(row);

        if (index.column() == 0)
          contact.title = value.toString();
        else if (index.column() == 8)
          contact.artist = value.toString();
        else if (index.column() == 1)
          contact.fileFormat = value.toString();
        else if (index.column() == 2)
          contact.length = value.toString();
        else if (index.column() == 3)
        {
            if(value==-1)
            {
                contact.subsong = "";
            }
            else
            {
                contact.subsong = value.toString();
            }
        }
        else if (index.column() == 4)
          contact.fullPath = value.toString();
        else if (index.column() == 5)
          contact.lengthInt = value.toInt()/1000;
        else if (index.column() == 6)
        {
          //cout << "setting bool to :" << value.toBool() << "\n";
          contact.playable = value.toBool();
        }
        else if (index.column() == 7)
        {
          //cout << "setting bool to :" << value.toBool() << "\n";
          contact.isPlaying = value.toBool();
        }
        else
          return false;

        items.replace(row, contact);
        emit(dataChanged(index, index));
        return true;
    }
    if (role == Qt::ForegroundRole)
    {
        return true;
    }


  }
