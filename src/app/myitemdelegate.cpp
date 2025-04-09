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

//    const int row = rIndex.row();
//    if (row == m_hoveredRow)
//    {
//        QColor highlightColor(m_root->getColorMain().left(7));
//        highlightColor.setAlpha(100 * m_opacity);
//        //QColor highlightColor = QColor(100, 150, 255, int(100 * m_opacity));
//        pPainter->fillRect(rOption.rect, highlightColor);
//    }
    QStyledItemDelegate::paint(pPainter, ViewOption, rIndex);
}

void MyItemDelegate::setHoveredRow(int row)
{
    if (m_hoveredRow != row) {
        m_hoveredRow = row;
        animateHighlight();
    }
}

void MyItemDelegate::setDragActive(bool active)
{
    if (m_dragActive != active) {
        m_dragActive = active;
        emit repaintRequested();  // request repaint
    }
}

void MyItemDelegate::animateHighlight() {
// Clean up previous animation
    if (m_animation) {
        m_animation->stop();
        m_animation->deleteLater();
    }

    m_opacity = 0.0;
    emit repaintRequested();

    // Start a new one
    m_animation = new QPropertyAnimation(this, "opacity");
    m_animation->setDuration(350);
    m_animation->setStartValue(0.0);
    m_animation->setEndValue(1.0);
    m_animation->setEasingCurve(QEasingCurve::InOutCubic);
    m_animation->start();
}

double MyItemDelegate::opacity() const
{
    return m_opacity;
}
void MyItemDelegate::setOpacity(double value)
{
    m_opacity = value;
    emit repaintRequested();
}
void MyItemDelegate::setDraggedRows(const QSet<int>& rows)
{
    m_draggedRows = rows;
    emit repaintRequested();
}
void MyItemDelegate::clearDraggedRows()
{
    m_draggedRows.clear();
    emit repaintRequested();
}