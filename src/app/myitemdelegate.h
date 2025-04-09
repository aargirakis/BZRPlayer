#ifndef MYITEMDELEGATE_H
#define MYITEMDELEGATE_H

#include "mainwindow.h"
#include <QStyledItemDelegate>
#include <QAbstractItemView>
#include <QPainter>
#include <QPropertyAnimation>

class MyItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    Q_PROPERTY(double opacity READ opacity WRITE setOpacity)
public:
    MyItemDelegate(QObject*);
    void paint(QPainter* pPainter, const QStyleOptionViewItem& rOption, const QModelIndex& rIndex) const override;
    void setHoveredRow(int row);
    void animateHighlight();
    double opacity() const;
    void setOpacity(double value);
    void setDraggedRows(const QSet<int>& rows);
    void clearDraggedRows();
    void setDragActive(bool active);

private:
    MainWindow* m_root;
    int m_hoveredRow = -1;
    double m_opacity = 0.0;
    QPropertyAnimation *m_animation = nullptr;
    QSet<int> m_draggedRows;
    bool m_dragActive = false;

signals:
    void repaintRequested();
};

#endif // MYITEMDELEGATE_H
