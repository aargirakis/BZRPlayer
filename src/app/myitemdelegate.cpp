#include "myitemdelegate.h"

MyItemDelegate::MyItemDelegate(QObject* parent)

{
    m_root = static_cast<MainWindow*>(parent);
}

void MyItemDelegate::paint(QPainter* pPainter, const QStyleOptionViewItem& rOption, const QModelIndex& rIndex) const
{
    QStyleOptionViewItem ViewOption(rOption);

    QColor ItemForegroundColor = rIndex.data(Qt::ForegroundRole).value<QColor>();
    if (ItemForegroundColor.isValid())
    {
        if (ItemForegroundColor != rOption.palette.color(QPalette::WindowText))
        {
            QColor c(m_root->getColorMain().left(7));
            QColor maintext(m_root->getColorMainText().left(7));
            maintext.setAlpha(128);
            if (ItemForegroundColor == c)
            {
                ViewOption.palette.setColor(QPalette::HighlightedText, QColor(m_root->getColorMain().left(7)));
            }
            else
            {
                ViewOption.palette.setColor(QPalette::HighlightedText, maintext);
            }
        }
    }

    QStyledItemDelegate::paint(pPainter, ViewOption, rIndex);
}
