#ifndef QLISTWIDGETCUSTOM_H
#define QLISTWIDGETCUSTOM_H

#include <QListWidget>
#include <QWidget>

class QListWidgetCustom : public QListWidget
{
public:
    explicit QListWidgetCustom(QWidget* parent = 0);

    ~QListWidgetCustom()
    {
    }
    void setDragBackgroundColor(QColor);
    void setDragTextColor(QColor );

protected:
    void startDrag(Qt::DropActions supportedActions) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    int m_dropLineRow = -1;
    QColor dragBackgroundColor;
    QColor dragTextColor;
};


#endif // QLISTWIDGETCUSTOM_H
