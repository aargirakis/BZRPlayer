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

    // ✅ This must be here:
    std::cout << "[DRAG] dragMoveEvent triggered\n";
    fflush(stdout);
    auto* proxy = qobject_cast<QSortFilterProxyModel*>(model());
    if (proxy) {
        auto* playlistModel = qobject_cast<PlaylistModel*>(proxy->sourceModel());
        if (playlistModel) {
            playlistModel->setDropTargetRow(m_dropLineRow);
            std::cout << "[DRAG] setDropTargetRow via proxy:" << m_dropLineRow << "\n";
            fflush(stdout);
        } else {
            std::cout << "[DRAG] sourceModel cast to PlaylistModel failed!\n";
            fflush(stdout);
        }
    } else {
        std::cout << "[DRAG] model is not a proxy\n";
        fflush(stdout);
    }

    QTableView::dragMoveEvent(event);
}


void DraggableTableView::dropEvent(QDropEvent *event) {
    setCurrentIndex(QModelIndex());
    if(m_animatedDelegate)
    {
        m_animatedDelegate->setDragActive(false);
    }
    m_dropLineRow = -1;
    viewport()->update();  // erase drop line
    QTableView::dropEvent(event);
}
void DraggableTableView::dragLeaveEvent(QDragLeaveEvent *event) {
    setCurrentIndex(QModelIndex());
    if(m_animatedDelegate)
    {
        m_animatedDelegate->setDragActive(false);
    }
    m_dropLineRow = -1;
    viewport()->update();  // erase drop line
    QTableView::dragLeaveEvent(event);
}
void DraggableTableView::startDrag(Qt::DropActions supportedActions) {
    QModelIndexList indexes = selectionModel()->selectedIndexes();
    if (indexes.isEmpty())
        return;

    // Collect unique rows (we only care about which rows are selected)
    QSet<int> uniqueRows;
    for (const QModelIndex& index : indexes)
        uniqueRows.insert(index.row());

    QList<int> sortedRows = uniqueRows.values();
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
    drag->setHotSpot(QPoint(10, 10));  // optional positioning

    drag->exec(Qt::MoveAction);
}
void DraggableTableView::setupAnimatedDelegate() {
    auto *delegate = new MyItemDelegate(m_root);
    setItemDelegate(delegate);

    connect(delegate, &MyItemDelegate::repaintRequested, this, [this]()
    {
        viewport()->update();  // Trigger repaint
    });

    m_animatedDelegate = delegate;
}
QPixmap DraggableTableView::createDragPixmap(const QList<int>& rows) {
    if (rows.isEmpty())
        return QPixmap();

    QFont font = this->font();
    QFontMetrics metrics(font);
    int rowHeight = sizeHintForRow(rows.first());
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

        painter.fillRect(rect, QColor(100, 150, 255, 200));
        painter.setPen(Qt::black);
        painter.drawText(rect.adjusted(5, 0, -5, 0), Qt::AlignLeft | Qt::AlignVCenter, text);
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