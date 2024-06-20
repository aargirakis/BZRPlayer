#ifndef MYITEMDELEGATE_H
#define MYITEMDELEGATE_H

#include "mainwindow.h"
#include <QStyledItemDelegate>
class MyItemDelegate: public QStyledItemDelegate
{
public:
    MyItemDelegate(QObject*);
    void paint(QPainter* pPainter, const QStyleOptionViewItem& rOption, const QModelIndex& rIndex) const override;
private:
    MainWindow* m_root;
};

#endif // MYITEMDELEGATE_H
