#include "ChipTrackerPatternView.h"

ChipTrackerPatternView::ChipTrackerPatternView(Tracker* parent, unsigned int channels, int scale)
    : AbstractPatternView(parent, channels, scale)
{
    rowNumberOffset = 0;
    octaveOffset = 12;
    m_font2 = QFont("Chiptracker Double Height");
    m_font2.setPixelSize(14);
    m_colorDefault = m_ColorRowNumber = m_ColorInstrument = m_ColorEffect = m_ColorParameter = m_ColorEffect2 =
        m_ColorParameter2 = m_ColorVolume = m_colorEmpty = QColor(33, 32, 255);
    m_font = QFont("Chiptracker");
    m_font.setPixelSize(7);
    m_fontWidth = 8;
    m_font.setStyleStrategy(QFont::NoAntialias);

    m_bitmapFont = BitmapFont("Chiptracker");
    m_fontWidth = 8;
    m_fontHeight = 7;
    m_bitmapFont2 = BitmapFont("Chiptracker Double Height");

    m_RowEnd = m_SeparatorRowNumber = m_SeparatorChannel = " ";
    m_RowLength = 40;
    m_xOffsetRow = 8;
    m_yOffsetCurrentRowBefore = -1;
    m_xChannelStart = 29;
    m_channelWidth = 70;
    m_channelLastWidth = 72;
    m_channelxSpace = 2;

    m_ibuttonPrevSampleWidth = 11;
    m_ibuttonPrevSampleHeight = 11;
    m_ibuttonPrevSampleX = 11;
    m_ibuttonPrevSampleY = 22;
    m_ibuttonNextSampleWidth = 11;
    m_ibuttonNextSampleHeight = 11;
    m_ibuttonNextSampleX = 0;
    m_ibuttonNextSampleY = 22;
}

QFont ChipTrackerPatternView::currentRowFont()
{
    return m_font2;
}

BitmapFont ChipTrackerPatternView::currentRowBitmapFont()
{
    return m_bitmapFont2;
}

ChipTrackerPatternView::~ChipTrackerPatternView()
{
}

void ChipTrackerPatternView::paintAbove(QPainter* painter, int height, int currentRow)
{
    QColor colorBase(115, 117, 115);
    QColor colorHilite(170, 173, 170);
    QColor colorShadow(66, 69, 66);

    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        int extra = 0;
        if (chan == m_channels - 1)
        {
            extra = 2;
        }
        painter->fillRect((30) + chan * 72, (height) - 3, (70 + extra), 1, colorHilite);
        painter->fillRect((29) + chan * 72, (height) - 3, 1, 1, colorBase);
        painter->fillRect((29) + chan * 72, 32, (70 + extra), 1, colorShadow);
    }
    painter->fillRect((2), 32, 25, 1, colorShadow);
    painter->fillRect((3), (height) - 3, 25, 1, colorHilite);
    painter->fillRect((1), (height) - 2, 318, 1, colorBase);
    painter->fillRect(1, (height) - 1, 319, 1, colorShadow);
    painter->fillRect(2, (height) - 3, 1, 1, colorBase);

    //1px antialias
    painter->fillRect(11, 32, 1, 1, colorBase);
    painter->fillRect(22, 32, 1, 1, colorBase);
    painter->fillRect(59, 32, 1, 1, colorBase);
}

void ChipTrackerPatternView::paintBelow(QPainter* painter, int height, int currentRow)
{
    int left = 3;

    QColor colorBase(115, 117, 115);
    QColor colorHilite(170, 173, 170);
    QColor colorShadow(66, 69, 66);
    QPen pen(colorBase);
    pen.setWidth(1);
    painter->setPen(pen);
    int topOffset = -4;


    //left border
    pen.setColor(colorHilite);
    painter->setPen(pen);
    painter->drawLine(((left - 2)), 0, (left - 3), height);
    pen.setColor(colorBase);
    painter->setPen(pen);
    painter->drawLine(((left - 1)), 0, (left - 2), height);
    pen.setColor(colorShadow);
    painter->setPen(pen);
    painter->drawLine(left - 0, 0, left - 0, height - 4);

    //1 pixel bottom antialiasing
    painter->fillRect((left - 1), height - 3, 1, 1, colorBase);
    //1 pixel bottom antialiasing
    painter->fillRect((left - 3), height - 1, 1, 1, colorBase);


    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        QColor colorRed(255, 0, 0);
        //channel dividers
        pen.setColor(colorHilite);
        painter->setPen(pen);
        painter->drawLine((left + 25 + chan * 72), 32, (left + 25 + chan * 72), height);
        pen.setColor(colorBase);
        painter->setPen(pen);
        painter->drawLine((left + 26 + chan * 72), 32, (left + 26 + chan * 72), height);
        pen.setColor(colorShadow);
        painter->setPen(pen);
        painter->drawLine((left + 27 + chan * 72), 32, (left + 27 + chan * 72), (height) - 4);
        //1 pixel top antialiasing
        painter->fillRect((left + 24 + chan * 72), 32, 1, 1, colorBase);
        //1 pixel bottom antialiasing
        painter->fillRect((left + 26 + chan * 72), (height) - 2, 1, 1, colorBase);
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
    painter->fillRect(317, 0, 1, 1, colorBase);
    painter->fillRect(317, 32, 1, 1, colorBase);

    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        //current row per channel

        int extra = 0;
        if (chan == m_channels - 1)
        {
            extra = 1;
        }

        //top hilite
        painter->fillRect(((left + 26)) + chan * 72, (height / 2) - 2 + (topOffset), (71 + extra), 2, colorHilite);
        //bottom shadow
        painter->fillRect(((left + 26)) + chan * 72, (height / 2) + 10 + (topOffset), (71 + extra), 2, colorShadow);
    }


    //current row left
    //top hilite
    painter->fillRect(((left - 1)), (height / 2) - 2 + (topOffset), 26, 2, colorHilite);
    //bottom shadow
    painter->fillRect(((left - 1)), (height / 2) + 10 + (topOffset), 26, 2, colorShadow);

    //main current row
    painter->fillRect(((left - 1)), (height / 2) - 4, 317, 10, colorBase);

    //right antialias
    painter->fillRect(317, (height / 2) - 6, 1, 14, colorBase);

    QColor colorGreenHiliteColor(0, 239, 0);
    QColor colorGreenBaseColor(0, 170, 0);
    QColor colorGreenShadowColor(0, 101, 0);
    //vumeters base
    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        painter->fillRect(((left + 30)) + chan * 72 + 22, (height / 2) - 2 + (topOffset), 2, 1, colorGreenHiliteColor);
        painter->fillRect(((left + 32)) + chan * 72 + 22, (height / 2) - 2 + (topOffset), 6, 1, colorGreenBaseColor);
        painter->fillRect(((left + 38)) + chan * 72 + 22, (height / 2) - 2 + (topOffset), 2, 1, colorGreenShadowColor);
    }
}
