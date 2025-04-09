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
public:
    MyItemDelegate(QObject*);
    void paint(QPainter* pPainter, const QStyleOptionViewItem& rOption, const QModelIndex& rIndex) const override;
    void setDragActive(bool active);
    void setMainTextColor(QColor);
    void setMainColor(QColor);

private:
    QSet<int> m_draggedRows;
    bool m_dragActive = false;
    QColor mainColor;
    QColor mainTextColor;


signals:
    void repaintRequested();
};

#endif // MYITEMDELEGATE_H
