#ifndef QDRAGGABLETABLEVIEW_H
#define QDRAGGABLETABLEVIEW_H
#include <QTableView>
#include "myitemdelegate.h"
class DraggableTableView : public QTableView
{
    Q_OBJECT

public:
    explicit DraggableTableView(QWidget* parent = 0);
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void startDrag(Qt::DropActions supportedActions) override;
    QPixmap createDragPixmap(const QList<int>& rows);
    void paintEvent(QPaintEvent* event) override;

private:
    MainWindow* m_root;
    int m_dropLineRow = -1;
    void setupDelegate();
protected:
    MyItemDelegate *m_Delegate = nullptr;


};
#endif // QDRAGGABLETABLEVIEW_H