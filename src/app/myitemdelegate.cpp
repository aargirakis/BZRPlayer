#include "myitemdelegate.h"

MyItemDelegate::MyItemDelegate(QObject* parent)

{
}

void MyItemDelegate::paint(QPainter* pPainter, const QStyleOptionViewItem& rOption, const QModelIndex& rIndex) const
{
    QStyleOptionViewItem ViewOption(rOption);


    if (auto ItemForegroundColor = rIndex.data(Qt::ForegroundRole).value<QColor>();
        ItemForegroundColor.isValid() && ItemForegroundColor != rOption.palette.color(QPalette::WindowText))
    {
        QColor mainTextColorDimmed = mainTextColor;
        mainTextColorDimmed.setAlpha(128);

        if (ItemForegroundColor == mainColor)
        {
            ViewOption.palette.setColor(QPalette::HighlightedText, mainColor);
        }
        else
        {
            ViewOption.palette.setColor(QPalette::HighlightedText, mainTextColorDimmed);
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
void MyItemDelegate::setMainColor(QColor c)
{
    mainColor = c;
}
void MyItemDelegate::setMainTextColor(QColor c)
{
    mainTextColor = c;
}
