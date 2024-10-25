#include "FastTracker26ChanPatternView.h"
#include "visualizers/tracker.h"

FastTracker26ChanPatternView::FastTracker26ChanPatternView(Tracker* parent, unsigned int channels, int scale)
    : FastTracker2PatternView(parent, channels, scale)
{
    m_font = QFont("Fasttracker 2");
    m_bitmapFont = BitmapFont("Fasttracker 2");
    m_font.setPixelSize(8);
    m_fontWidth = 8;
    m_bitmapFontEffects = BitmapFont("Fasttracker 2");
    m_fontEffects = QFont("Fasttracker 2");
    m_fontEffects.setPixelSize(8);
    m_bitmapFontParameters = BitmapFont("Fasttracker 2");
    m_bitmapFontInstrument = BitmapFont("Fasttracker 2");
    m_fontParameters = QFont("Fasttracker 2");
    m_fontParameters.setPixelSize(8);
    m_fontWidthEffects = 8;
    m_fontWidthInstrument = 8;
    m_fontWidthEffects = 8;
    m_fontWidthParameters = 8;
    m_fontWidthSeparatorNote = 8;
    m_SeparatorNote = "";
    m_SeparatorVolume = "";
    m_RowLength = 4 + (m_channels * 12) + 3;

    m_xChannelStart = 28;
    m_channelWidth = 94;
    m_channelxSpace = 2;
}

FastTracker26ChanPatternView::~FastTracker26ChanPatternView()
{
}

void FastTracker26ChanPatternView::paintAbove(QPainter* painter, int height, int currentRow)
{
    QColor colorBase(73, 117, 130);
    QColor colorHilite(138, 219, 243);
    QColor colorShadow(24, 40, 44);

    painter->setPen(QColor(255, 255, 255));

    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        //bottom channels
        painter->fillRect((29) + chan * 96, (height) - 3, 94, 1, colorHilite);
        //top channels
        painter->fillRect((29) + chan * 96, 31, 94, 1, colorShadow);
        //channel number
        Tracker* t = (Tracker*)this->parent();
        t->drawText(QString::number(chan), painter, (28) + chan * 96 * m_fontWidth / 8, 43);

        //black line
        painter->fillRect((28) + chan * 96, 33, 1, 10, QColor(0, 0, 0));
    }
    //top
    painter->fillRect((1), 30, 53 + (m_channels * 96), 1, colorBase);
    painter->fillRect((1), 29, 52 + (m_channels * 96), 1, colorHilite);
    //bottom
    painter->fillRect((1), (height) - 2, 26 + (m_channels * 96), 1, colorBase);
    painter->fillRect(0, (height) - 1, 55 + (m_channels * 96), 1, colorShadow);

    //bottom left and right
    painter->fillRect((2), (height) - 3, 25, 1, colorHilite);
    painter->fillRect(29 + (m_channels * 96), (height) - 3, 24, 1, colorHilite);
    painter->fillRect(29 + (m_channels * 96), (height) - 2, 24, 1, colorBase);

    //top left
    painter->fillRect((2), 31, 25, 1, colorShadow);
    //top right
    painter->fillRect(29 + (m_channels * 96), 31, 25, 1, colorShadow);
}

void FastTracker26ChanPatternView::paintBelow(QPainter* painter, int height, int currentRow)
{
    int left = 3;

    QColor colorBase(73, 117, 130);
    QColor colorHilite(138, 219, 243);
    QColor colorShadow(24, 40, 44);
    QPen pen(colorBase);
    pen.setWidth(1);
    painter->setPen(pen);
    int topOffset = -4;


    //left border
    pen.setColor(colorHilite);
    painter->setPen(pen);
    painter->drawLine(((left - 2)), 29, ((left - 2)), height);
    pen.setColor(colorBase);
    painter->setPen(pen);
    painter->drawLine(((left - 1)), 29, ((left - 1)), height);
    pen.setColor(colorShadow);
    painter->setPen(pen);
    painter->drawLine(((left)), 29, ((left)), height - 4);

    for (unsigned int chan = 0; chan <= m_channels; chan++)
    {
        //channel dividers
        pen.setColor(colorHilite);
        painter->setPen(pen);
        painter->drawLine((left + 24 + chan * 96), 29, (left + 24 + chan * 96), height);
        pen.setColor(colorBase);
        painter->setPen(pen);
        painter->drawLine((left + 25 + chan * 96), 29, (left + 25 + chan * 96), height);
        pen.setColor(colorShadow);
        painter->setPen(pen);
        painter->drawLine((left + 26 + chan * 96), 29, (left + 26 + chan * 96), (height) - 2);
    }

    //right border
    pen.setColor(colorHilite);
    painter->setPen(pen);
    painter->drawLine((54) + (m_channels * 96), 29, (54) + (m_channels * 96), height);
    pen.setColor(colorBase);
    painter->setPen(pen);
    painter->drawLine((55) + (m_channels * 96), 29, (55) + (m_channels * 96), height);
    pen.setColor(colorShadow);
    painter->setPen(pen);
    painter->drawLine((56) + (m_channels * 96), 29, (56) + (m_channels * 96), height);

    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        //top hilite
        painter->fillRect(((left + 26)) + chan * 96, (height / 2) - 6 + (topOffset), 94, 1, colorHilite);
        //bottom shadow
        painter->fillRect(((left + 26)) + chan * 96, (height / 2) + 4 + (topOffset), 94, 1, colorShadow);
    }


    QColor colorRed(255, 0, 0);

    //current row left
    //top hilite
    painter->fillRect(((left - 1)), (height / 2) - 6 + (topOffset), 24, 1, colorHilite);
    //bottom shadow
    painter->fillRect(((left - 1)), (height / 2) + 4 + (topOffset), 25, 1, colorShadow);

    //current row right
    //top hilite
    painter->fillRect(((left + 25)) + m_channels * 96, (height / 2) - 6 + (topOffset), 25, 1, colorHilite);
    //bottom shadow
    painter->fillRect(((left + 26)) + m_channels * 96, (height / 2) + 4 + (topOffset), 25, 1, colorShadow);

    //main current row
    painter->fillRect(((1)), (height / 2) - 9, (53) + (m_channels * 96), 9, colorBase);
}
