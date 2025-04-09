#include "qlistwidgetcustom.h"
#include "qevent.h"
#include <iostream>
#include <QMimeData>
#include <QDrag>
#include <QPainter>

QListWidgetCustom::QListWidgetCustom(QWidget* parent)
        : QListWidget(parent)
{
    setAcceptDrops(true);
    setDragEnabled(true);
    setDropIndicatorShown(false); // we'll draw manually
    setDragDropMode(QAbstractItemView::DragDrop);
    setDefaultDropAction(Qt::MoveAction);
}

void QListWidgetCustom::startDrag(Qt::DropActions supportedActions)
{
    if (currentRow() == 0) {
        qDebug() << "Top row is locked. Not draggable.";
        return; // Don't start drag
    }
    QListWidgetItem* item = currentItem();
    if (!item) return;

    QByteArray encoded;
    QDataStream stream(&encoded, QIODevice::WriteOnly);
    stream << currentRow(); // Save row index

    QMimeData* mimeData = new QMimeData;
    mimeData->setData("application/playlist.text.list", encoded);

    QDrag* drag = new QDrag(this);
    drag->setMimeData(mimeData);

    int rowHeight = sizeHintForRow(0);
    int width = viewport()->width();
    QRect rect(0, 0, width, rowHeight);
    QPixmap pixmap(viewport()->width(), sizeHintForRow(currentRow()));
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QColor bg = dragBackgroundColor;
    bg.setAlpha((150));
    painter.fillRect(pixmap.rect(), bg);
    painter.setPen(dragTextColor);
    painter.drawText(rect.adjusted(25, 0, -5, 0), Qt::AlignLeft | Qt::AlignVCenter, item->text());
    painter.end();

    drag->setPixmap(pixmap);
    drag->setHotSpot(QPoint(10, 10));

    drag->exec(Qt::MoveAction);
}

void QListWidgetCustom::dragMoveEvent(QDragMoveEvent* event)
{
    QModelIndex index = indexAt(event->pos());
    int newDropRow;

    if (index.isValid()) {
        QRect rect = visualRect(index);
        int midpoint = rect.top() + rect.height() / 2;
        newDropRow = (event->pos().y() < midpoint) ? index.row() : index.row() + 1;
    } else {
        newDropRow = count();
    }

    // Never allow drop before row 0
    if (newDropRow <= 0)
        newDropRow = 1;

    if (newDropRow != m_dropLineRow) {
        m_dropLineRow = newDropRow;
        viewport()->update();
    }

    event->acceptProposedAction();
}

void QListWidgetCustom::dropEvent(QDropEvent* event)
{
    QByteArray encoded = event->mimeData()->data("application/playlist.text.list");
    QDataStream stream(&encoded, QIODevice::ReadOnly);
    int sourceRow;
    stream >> sourceRow;

    int targetRow = m_dropLineRow;

    if (targetRow <= 0)
        targetRow = 1;

    // Adjust target if dragging downwards past itself
    if (sourceRow < targetRow)
        targetRow -= 1;

    if (sourceRow == targetRow) {
        // No move needed
        event->ignore();
        m_dropLineRow = -1;
        viewport()->update();
        return;
    }

    // Take and reinsert item
    QListWidgetItem* item = takeItem(sourceRow);
    insertItem(targetRow, item);
    setCurrentRow(targetRow);

    event->acceptProposedAction();

    m_dropLineRow = -1;
    viewport()->update();
}

void QListWidgetCustom::dragLeaveEvent(QDragLeaveEvent* event)
{
    QListWidget::dragLeaveEvent(event);
    m_dropLineRow = -1;
    viewport()->update();
}
void QListWidgetCustom::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat("application/playlist.text.list")) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}
void QListWidgetCustom::paintEvent(QPaintEvent* event)
{
    QListWidget::paintEvent(event);

    if (m_dropLineRow < 0 || m_dropLineRow > count())
        return;

    QPainter painter(viewport());
    QPen pen(dragBackgroundColor); // custom color for drop line
    pen.setWidth(2);
    painter.setPen(pen);

    int y;
    if (m_dropLineRow < count()) {
        QRect rect = visualRect(model()->index(m_dropLineRow, 0));
        y = rect.top();
    } else if (count() > 0) {
        QRect lastRect = visualRect(model()->index(count() - 1, 0));
        y = lastRect.bottom();
    } else {
        y = 0;
    }

    painter.drawLine(0, y, viewport()->width(), y);
}
void QListWidgetCustom::setDragBackgroundColor(QColor c)
{
    dragBackgroundColor=c;
}
void QListWidgetCustom::setDragTextColor(QColor c)
{
    dragTextColor=c;
}