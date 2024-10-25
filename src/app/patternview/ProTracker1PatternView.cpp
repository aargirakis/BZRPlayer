#include "ProTracker1PatternView.h"

ProTracker1PatternView::ProTracker1PatternView(Tracker* parent, unsigned int channels, int scale)
    : AbstractPatternView(parent, channels, scale)
{
    rowNumberOffset = 0;
    octaveOffset = 48;
    m_font2 = QFont("Protracker 1.0 Double Height");

    m_font2.setPixelSize(14);
    m_font = QFont("Protracker 1.0");
    m_bitmapFont = BitmapFont("Protracker 1.0");
    m_bitmapFont2 = BitmapFont("Protracker 1.0 Double Height");
    m_font.setPixelSize(7);
    m_fontWidth = 8;
    m_fontHeight = 7;
    m_font.setStyleStrategy(QFont::NoAntialias);

    m_colorDefault = m_ColorRowNumber = m_ColorInstrument = m_ColorEffect = m_ColorParameter = m_ColorEffect2 =
        m_ColorParameter2 = m_ColorVolume = m_colorEmpty = QColor(50, 50, 255);


    m_RowEnd = m_SeparatorRowNumber = m_SeparatorChannel = " ";
    m_RowLength = 40;
    m_xOffsetRow = 8;
    //for dimming channels
    m_xChannelStart = 30;
    m_channelWidth = 69;
    m_channelLastWidth = 71;
    m_channelxSpace = 3;

    m_ibuttonPrevSampleWidth = 11;
    m_ibuttonPrevSampleHeight = 11;
    m_ibuttonPrevSampleX = 11;
    m_ibuttonPrevSampleY = 22;
    m_ibuttonNextSampleWidth = 11;
    m_ibuttonNextSampleHeight = 11;
    m_ibuttonNextSampleX = 0;
    m_ibuttonNextSampleY = 22;
}

BitmapFont ProTracker1PatternView::currentRowBitmapFont()
{
    return m_bitmapFont2;
}

QFont ProTracker1PatternView::currentRowFont()
{
    return m_font2;
}

void ProTracker1PatternView::paintAbove(QPainter* painter, int height, int currentRow)
{
    QColor colorBase(136, 136, 136);
    QColor colorHilite(187, 187, 187);
    QColor colorShadow(85, 85, 85);

    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        int extra = 0;
        if (chan == m_channels - 1)
        {
            extra = 2;
        }
        painter->fillRect((30) + chan * 72, (height) - 3, (70 + extra), 1, colorHilite);
        painter->fillRect((29) + chan * 72, 32, (70 + extra), 1, colorShadow);
    }
    painter->fillRect((2), 32, 25, 1, colorShadow);
    painter->fillRect((3), (height) - 3, 25, 1, colorHilite);
    painter->fillRect((1), (height) - 2, 318, 1, colorBase);
    painter->fillRect(1, (height) - 1, 319, 1, colorShadow);

    //1px antialias
    painter->fillRect(11, 32, 1, 1, colorBase);
    painter->fillRect(22, 32, 1, 1, colorBase);
    painter->fillRect(45, 32, 1, 1, colorBase);
}

void ProTracker1PatternView::paintBelow(QPainter* painter, int height, int currentRow)
{
    int left = 4;

    QColor colorBase(136, 136, 136);
    QColor colorHilite(187, 187, 187);
    QColor colorShadow(85, 85, 85);
    QPen pen(colorBase);
    pen.setWidth(1);
    painter->setPen(pen);
    int topOffset = -4;


    //left border
    pen.setColor(colorHilite);
    painter->setPen(pen);
    painter->drawLine(left - 3, 0, left - 3, height);
    pen.setColor(colorBase);
    painter->setPen(pen);
    painter->drawLine(left - 2, 0, left - 2, height);
    pen.setColor(colorShadow);
    painter->setPen(pen);
    painter->drawLine(left - 1, 0, left - 1, height - 4);
    QColor colorRed(255, 0, 0);
    //1 pixel bottom antialiasing
    painter->fillRect(((left - 2)), height - 3, 1, 1, colorBase);
    painter->fillRect(((left - 4)), height - 1, 1, 1, colorBase);

    //current row left
    //top hilite
    painter->fillRect(((left + 0)), (height / 2) - 2 + (topOffset), 21, 2, colorHilite);
    //bottom shadow
    painter->fillRect(((left + 1)), (height / 2) + 10 + (topOffset), 21, 2, colorShadow);

    //    //left hilite
    painter->fillRect(((left + 0)), (height / 2) - 0 + (topOffset), (1), 10, colorHilite);

    //    //left antialias
    painter->fillRect(((left + 0)), (height / 2) + 10 + (topOffset), (1), 2, colorBase);

    //    //right shadow
    painter->fillRect(((left + 21)), (height / 2) - 0 + (topOffset), (1), 10, colorShadow);

    //    //right antialias
    painter->fillRect(((left + 21)), (height / 2) - 2 + (topOffset), (1), 2, colorBase);

    //main
    painter->fillRect(((left + 1)), (height / 2) - 4, 20, 10, colorBase);


    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        //channel dividers
        pen.setColor(colorHilite);
        painter->setPen(pen);
        painter->drawLine((left + 24 + chan * 72), 0, (left + 24 + chan * 72), height);
        pen.setColor(colorBase);
        painter->setPen(pen);
        painter->drawLine((left + 25 + chan * 72), 0, (left + 25 + chan * 72), height);
        pen.setColor(colorShadow);
        painter->setPen(pen);
        painter->drawLine((left + 26 + chan * 72), 0, (left + 26 + chan * 72), (height) - 4);
        //1 pixel top antialiasing
        painter->fillRect((left + 23 + chan * 72), 32, 1, 1, colorBase);
        painter->fillRect((left + 25 + chan * 72), (height) - 3, 1, 1, colorBase);

        //current row per channel

        int extra = 0;
        if (chan == m_channels - 1)
        {
            extra = 2;
        }

        //top hilite
        painter->fillRect(((left + 27)) + chan * 72, (height / 2) - 2 + (topOffset), (66 + extra), 2, colorHilite);
        //bottom shadow
        painter->fillRect(((left + 28)) + chan * 72, (height / 2) + 10 + (topOffset), (66 + extra), 2, colorShadow);

        //left hilite
        painter->fillRect(((left + 27)) + chan * 72, (height / 2) - 0 + (topOffset), (1), 10, colorHilite);

        //left antialias
        painter->fillRect(((left + 27)) + chan * 72, (height / 2) + 10 + (topOffset), (1), 2, colorBase);

        //right shadow
        painter->fillRect(((left + 93 + extra)) + chan * 72, (height / 2) - 0 + (topOffset), (1), 10, colorShadow);

        //right antialias
        painter->fillRect(((left + 93 + extra)) + chan * 72, (height / 2) - 2 + (topOffset), (1), 2, colorBase);

        //main
        painter->fillRect(((left + 28)) + chan * 72, (height / 2) - 4, (65 + extra), 10, colorBase);
    }

    //right border
    pen.setColor(colorHilite);
    painter->setPen(pen);
    painter->drawLine(318, 0, 318, height);
    pen.setColor(colorBase);
    painter->setPen(pen);
    painter->drawLine(319, 0, 319, height);
    pen.setColor(colorShadow);
    painter->setPen(pen);
    painter->drawLine(320, 0, 320, height);
    //1 pixel antialiasing
    painter->fillRect(317, 32, 1, 1, colorBase);
}

ProTracker1PatternView::~ProTracker1PatternView()
{
}
