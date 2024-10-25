#include "UltimateSoundTrackerPatternView.h"

UltimateSoundTrackerPatternView::UltimateSoundTrackerPatternView(Tracker* parent, unsigned int channels, int scale)
    : AbstractPatternView(parent, channels, scale)
{
    rowNumberOffset = 0;
    octaveOffset = 48;
    m_font2 = QFont("UST Double Height");
    m_font2.setPixelSize(14);
    m_font = QFont("Ultimate Soundtracker");
    m_font.setPixelSize(7);

    m_bitmapFont = BitmapFont("Ultimate Soundtracker");
    m_bitmapFont2 = BitmapFont("Ultimate Soundtracker Double Height");
    m_fontWidth = 8;
    m_fontHeight = 7;

    m_fontWidth = 8;
    m_emptyInstrument = "0";
    m_SeparatorNote = "'";
    instrumentPad = false;
    m_colorCurrentRowBackground = QColor(156, 117, 82);
    m_colorDefault = m_ColorEffect = m_ColorRowNumber = m_ColorInstrument = m_ColorParameter = m_ColorVolume =
        m_colorEmpty = QColor(0, 101, 156);
    m_RowEnd = m_SeparatorRowNumber = m_SeparatorChannel = "'";
    m_RowLength = 40;
    m_xOffsetRow = 9;

    //for dimming channels
    m_xChannelStart = 30;
    m_channelWidth = 69;
    m_channelLastWidth = 71;
    m_channelxSpace = 3;

    m_ibuttonPrevSampleWidth = 9;
    m_ibuttonPrevSampleHeight = 7;
    m_ibuttonPrevSampleX = 14;
    m_ibuttonPrevSampleY = 24;
    m_ibuttonNextSampleWidth = 9;
    m_ibuttonNextSampleHeight = 7;
    m_ibuttonNextSampleX = 2;
    m_ibuttonNextSampleY = 24;
}

QFont UltimateSoundTrackerPatternView::currentRowFont()
{
    return m_font2;
}

BitmapFont UltimateSoundTrackerPatternView::currentRowBitmapFont()
{
    return m_bitmapFont2;
}

void UltimateSoundTrackerPatternView::paintAbove(QPainter* painter, int height, int currentRow)
{
    //bottom

    QColor colorBase(156, 117, 82);
    QColor colorHilite(173, 138, 99);
    QColor colorShadow(82, 48, 16);
    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        int extra = 0;
        if (chan == m_channels - 1)
        {
            extra = 2;
        }
        painter->fillRect((29) + chan * 72, (height) - 3, (70 + extra), 1, colorHilite);
        painter->fillRect((29) + chan * 72, 32, (70 + extra), 1, colorShadow);
    }
    painter->fillRect((2), 32, 25, 1, colorShadow);
    painter->fillRect((2), (height) - 3, 26, 1, colorHilite);
    painter->fillRect((1), (height) - 2, 318, 1, colorBase);
    painter->fillRect(0, (height) - 1, 319, 1, colorShadow);
}

void UltimateSoundTrackerPatternView::paintBelow(QPainter* painter, int height, int currentRow)
{
    int left = 3;

    QColor colorBase(156, 117, 82);
    QColor colorHilite(173, 138, 99);
    QColor colorShadow(82, 48, 16);
    QColor colorGreenHiliteColor(0, 173, 0);
    QColor colorGreenBaseColor(0, 140, 0);
    QColor colorGreenShadowColor(0, 99, 0);
    QPen pen(colorBase);
    pen.setWidth(1);
    painter->setPen(pen);
    int topOffset = -4;


    //left border
    pen.setColor(colorHilite);
    painter->setPen(pen);
    painter->drawLine(((left - 2)), 0, ((left - 2)), height);
    pen.setColor(colorBase);
    painter->setPen(pen);
    painter->drawLine(((left - 1)), 0, ((left - 1)), height);
    pen.setColor(colorShadow);
    painter->setPen(pen);
    painter->drawLine(((left - 0)), 0, ((left - 0)), height);


    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        //channel dividers
        pen.setColor(colorHilite);
        painter->setPen(pen);
        painter->drawLine((left + 25 + chan * 72), 0, (left + 25 + chan * 72), height);
        pen.setColor(colorBase);
        painter->setPen(pen);
        painter->drawLine((left + 26 + chan * 72), 0, (left + 26 + chan * 72), height);
        pen.setColor(colorShadow);
        painter->setPen(pen);
        painter->drawLine((left + 27 + chan * 72), 0, (left + 27 + chan * 72), height);

        //current row per channel

        int extra = 0;
        if (chan == m_channels - 1)
        {
            extra = 2;
        }
        //top hilite
        painter->fillRect(((left + 26)) + chan * 72, (height / 2) - 2 + (topOffset), (70 + extra), 2, colorHilite);
        //bottom shadow
        painter->fillRect(((left + 26)) + chan * 72, (height / 2) + 10 + (topOffset), (70 + extra), 2, colorShadow);
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


    topOffset = -2;
    painter->fillRect(((left - 2)), (height / 2) - 2 + (topOffset), 317, 10, colorBase);
    //vumeters base
    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        painter->fillRect(((left + 31)) + chan * 72 + 22, (height / 2) - 4 + (topOffset), 2, 1, colorGreenHiliteColor);
        painter->fillRect(((left + 33)) + chan * 72 + 22, (height / 2) - 4 + (topOffset), 4, 1, colorGreenBaseColor);
        painter->fillRect(((left + 37)) + chan * 72 + 22, (height / 2) - 4 + (topOffset), 2, 1, colorGreenShadowColor);
    }
    //leftmost current row

    topOffset = -4;
    painter->fillRect(((left - 1)), (height / 2) - 2 + (topOffset), 25, 2, colorHilite);
    painter->fillRect(((left - 1)), (height / 2) + 10 + (topOffset), 25, 2, colorShadow);
}

UltimateSoundTrackerPatternView::~UltimateSoundTrackerPatternView()
{
}
