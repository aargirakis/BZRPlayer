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
    if (m_dragActive) {
        // Remove *all* visual states
        ViewOption.state &= ~QStyle::State_MouseOver;
        ViewOption.state &= ~QStyle::State_Selected;
        ViewOption.state &= ~QStyle::State_HasFocus;
        ViewOption.state &= ~QStyle::State_Active;
        ViewOption.state &= ~QStyle::State_Enabled;
    }

    QStyledItemDelegate::paint(pPainter, ViewOption, rIndex);
}



void MyItemDelegate::setDragActive(bool active)
{
    if (m_dragActive != active) {
        m_dragActive = active;
        emit repaintRequested();  // request repaint
    }
}

