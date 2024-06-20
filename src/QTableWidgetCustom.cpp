#include "QTableWidgetCustom.h"
#include "qdebug.h"
#include "qmimedata.h"
#include <QApplication>

QTableWidgetCustom::QTableWidgetCustom(QWidget *parent)
    : QTableWidget(parent)
{
    setDropIndicatorShown(true);
    setDragEnabled(true);
    setDragDropMode(DragDrop);
}
void QTableWidgetCustom::dragEnterEvent(QDragEnterEvent *event)
{
//    qDebug() << "drag!";
//    if (event->mimeData()->hasFormat("text/uri-list"))
//    {
//        qDebug() << "drag accepted!";
//        event->acceptProposedAction();
//    }
}
void QTableWidgetCustom::dropEvent(QDropEvent *event)
{

//    QList<QUrl> list = event->mimeData()->urls();

    //addSong(list,0,ui->listWidget->currentItem()->text(),true);

}
void QTableWidgetCustom::keyPressEvent(QKeyEvent *event)
{
       switch(event->key()) {
           case Qt::Key_Left:
           {
                //do nothing, we use this for seeking
               break;
           }
           case Qt::Key_Right:
           {
                //do nothing, we use this for seeking
               break;
           }
//       case Qt::Key_Tab: {
//           if(currentIndex().row() != model()->rowCount())
//               selectRow(currentIndex().row() + 1);
//           break;
//       }
//       case Qt::Key_End: {
//           clearSelection();
//            selectRow(model()->rowCount()-1);
//            break;
//       }
//       case Qt::Key_Home: {
//           if(QApplication::keyboardModifiers()==(Qt::ShiftModifier))
//           {
//               QModelIndexList indexes = this->selectionModel()->selectedRows();

//                for(int i=0;i<indexes.first().row();i++)
//                {
//                    qDebug() << i ;
//                    selectRow(i);
//                }
//           }
//           else
//           {
//            selectRow(0);
//           }
//            break;
//       }
       default: QTableView::keyPressEvent(event);
       }
   }
