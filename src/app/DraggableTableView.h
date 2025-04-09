#ifndef QDRAGGABLETABLEVIEW_H
#define QDRAGGABLETABLEVIEW_H

#include "myitemdelegate.h"
class DraggableTableView : public QTableView
{
    Q_OBJECT

public:
    explicit DraggableTableView(QWidget* parent = nullptr);
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void startDrag(Qt::DropActions supportedActions) override;
    QPixmap createDragPixmap(const QList<int>& rows) const;
    void paintEvent(QPaintEvent* event) override;
    void setDragBackgroundColor(QColor);
    void setDragTextColor(QColor);
    void setupDelegate();

private:
    int m_dropLineRow = -1;
    QColor dragBackgroundColor;
    QColor dragTextColor;

protected:
    MyItemDelegate *m_Delegate = nullptr;


};
#endif // QDRAGGABLETABLEVIEW_H