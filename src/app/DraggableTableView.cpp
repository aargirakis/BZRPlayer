#include "DraggableTableView.h"
#include "myitemdelegate.h"
#include "playlistmodel.h"
#include <QDebug>
#include <QDragMoveEvent>
#include <QDrag>
#include <iostream>
#include <QSortFilterProxyModel>

DraggableTableView::DraggableTableView(QWidget* parent)
        : QTableView(parent)
{
    setStyleSheet("QTableView::item:focus { outline: none; border: none; }");
    m_root = static_cast<MainWindow*>(parent);
    setWordWrap(false);
    setShowGrid(false);
    setFrameShape(QFrame::NoFrame);
    setFrameShadow(QFrame::Plain);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionBehavior(QAbstractItemView::SelectRows);

    setDragEnabled(true);
    setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setDropIndicatorShown(false);
    setDefaultDropAction(Qt::MoveAction);
    setupDelegate();

}


void DraggableTableView::dragMoveEvent(QDragMoveEvent* event)
{
    QModelIndex index = indexAt(event->pos());
    int newDropRow;

    if (index.isValid()) {
        QRect rect = visualRect(index);
        int midpoint = rect.top() + rect.height() / 2;
        newDropRow = (event->pos().y() < midpoint) ? index.row() : index.row() + 1;
    } else {
        newDropRow = model()->rowCount();
    }

    if (newDropRow != m_dropLineRow) {
        m_dropLineRow = newDropRow;
        viewport()->update();
    }

    auto* proxy = qobject_cast<QSortFilterProxyModel*>(model());
    auto* playlistModel = qobject_cast<PlaylistModel*>(proxy->sourceModel());
    playlistModel->setDropTargetRow(m_dropLineRow);
    QTableView::dragMoveEvent(event);
}


void DraggableTableView::dropEvent(QDropEvent *event) {
    setCurrentIndex(QModelIndex());
    m_Delegate->setDragActive(false);
    m_dropLineRow = -1;
    viewport()->update();  // erase drop line
    QTableView::dropEvent(event);
}
void DraggableTableView::dragLeaveEvent(QDragLeaveEvent *event) {
    setCurrentIndex(QModelIndex());
    m_Delegate->setDragActive(false);
    m_dropLineRow = -1;
    viewport()->update();  // erase drop line
    QTableView::dragLeaveEvent(event);
}
void DraggableTableView::startDrag(Qt::DropActions supportedActions) {
    QModelIndexList indexes = selectionModel()->selectedIndexes();
    if (indexes.isEmpty())
        return;

    QSet<int> selectedRows;
    for (const QModelIndex& index : indexes)
        selectedRows.insert(index.row());

    //We need to sort the rows, otherwise it will be the picking order
    QList<int> sortedRows = selectedRows.values();
    std::sort(sortedRows.begin(), sortedRows.end());

    // Serialize the row numbers into MIME data
    QByteArray encoded;
    QDataStream stream(&encoded, QIODevice::WriteOnly);
    for (int row : sortedRows)
        stream << row;

    QMimeData* mimeData = new QMimeData();
    mimeData->setData("application/vnd.text.list", encoded);

    // Create a drag pixmap showing the dragged rows
    QPixmap pixmap = createDragPixmap(sortedRows);

    // Launch drag
    QDrag* drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(pixmap);
    drag->setHotSpot(QPoint(10, 10));

    drag->exec(Qt::MoveAction);
}
void DraggableTableView::setupDelegate() {
    auto *delegate = new MyItemDelegate(m_root);
    setItemDelegate(delegate);
    m_Delegate = delegate;
}
QPixmap DraggableTableView::createDragPixmap(const QList<int>& rows) {
    if (rows.isEmpty())
        return QPixmap();

    QFont font = this->font();
    QFontMetrics metrics(font);
    int rowHeight = sizeHintForRow(rows.first())*2;
    int width = viewport()->width();
    int height = rowHeight * rows.count();

    QPixmap pixmap(width, height);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setFont(font);

    int y = 0;
    for (int row : rows) {
        QRect rect(0, y, width, rowHeight);
        QModelIndex index = model()->index(row, 0);  // show first column
        QString text = model()->data(index).toString();

        QColor backgroundColor(m_root->getColorMain().left(7));
        backgroundColor.setAlpha((150));
        painter.fillRect(rect, backgroundColor);
        QColor textColor(m_root->getColorMainText().left(7));
        painter.setPen(textColor);
        painter.drawText(rect.adjusted(25, 0, -5, 0), Qt::AlignLeft | Qt::AlignVCenter, text);
        y += rowHeight;
    }

    painter.end();
    return pixmap;
}
void DraggableTableView::paintEvent(QPaintEvent* event) {
    QTableView::paintEvent(event);

    if (m_dropLineRow < 0 || m_dropLineRow > model()->rowCount())
        return;

    QPainter painter(viewport());
    QColor mainColor(m_root->getColorMain().left(7));
    QPen pen(mainColor);
    pen.setWidth(2);
    painter.setPen(pen);

    int y;
    if (m_dropLineRow < model()->rowCount()) {
        QRect rect = visualRect(model()->index(m_dropLineRow, 0));
        y = rect.top();
    } else if (model()->rowCount() > 0) {
        // Place drop line just below the bottom of the last visible row
        QRect lastRect = visualRect(model()->index(model()->rowCount() - 1, 0));
        y = lastRect.bottom() + 1;
    } else {
        // No rows in the model — just show it at the top
        y = 0;
    }

    painter.drawLine(0, y, viewport()->width(), y);
}