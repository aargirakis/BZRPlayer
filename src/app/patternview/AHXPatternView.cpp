#include "AHXPatternView.h"

AHXPatternView::AHXPatternView(Tracker* parent, unsigned int channels, int scale)
    : AbstractPatternView(parent, channels, scale)
{
    instrumentPad = true;
    instrumentHex = false;
    m_font = QFont("AHX");
    m_font.setPixelSize(8);
    m_fontWidth = 8;

    m_bitmapFont = BitmapFont("AHX");
    m_fontWidth = 8;
    m_fontHeight = 8;
    m_bitmapFont3 = BitmapFont("AHX Thin");

    rowNumberOffset = 0;
    m_colorDefault = m_ColorEffect = m_ColorInstrument = m_ColorParameter = m_ColorRowNumber = m_ColorVolume =
        m_colorEmpty = QColor(206, 231, 231);
    m_colorCurrentRowForeground = QColor(255, 255, 255);
    m_colorCurrentRowBackground = QColor(82, 89, 99);
    m_SeparatorRowNumber = " ";
    m_RowEnd = m_SeparatorChannel = " ";
    m_RowLength = 40;
    m_xOffsetRow = 14;
    m_xOffsetSeparatorRowNumber = -3;

    //for dimming channels
    m_xChannelStart = 32;
    m_channelWidth = 70;
    m_channelLastWidth = m_channelWidth;
    m_channelxSpace = 2;
    m_topHeight = 44;
    m_bottomFrameHeight = 2;
}

BitmapFont AHXPatternView::infoFont()
{
    return m_bitmapFont3;
}

void AHXPatternView::paintAbove(QPainter* painter, int height, int currentRow)
{
    QColor colorHilite = QColor(255, 255, 255);
    QColor colorShadow = QColor(82, 89, 99);
    QPen pen(colorHilite);
    pen.setWidth(1);
    //top
    pen.setColor(colorShadow);
    painter->setPen(pen);
    painter->drawLine(14, 43, 319, 43);

    //bottom
    pen.setColor(colorHilite);
    painter->setPen(pen);
    painter->drawLine(13, height - 2, 319, height - 2);
    pen.setColor(colorShadow);
    painter->setPen(pen);
    painter->drawLine(14, height - 1, 319, height - 1);


    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        //channel dividers
        pen.setColor(colorHilite);
        painter->setPen(pen);
        painter->drawLine((31 + chan * 72), 43, (31 + chan * 72), height - 2);
        pen.setColor(colorShadow);
        painter->setPen(pen);
        painter->drawLine((32 + chan * 72), 43, (32 + chan * 72), height - 3);
        painter->setPen(pen);
    }

    //top
    pen.setColor(colorHilite);
    painter->setPen(pen);
    painter->drawLine(13, 42, 319, 42);


    //right border
    pen.setColor(colorHilite);
    painter->setPen(pen);
    painter->drawLine((319), 43, (319), height - 3);
    pen.setColor(colorShadow);
    painter->setPen(pen);
    painter->drawLine((320), 0, (320), height - 1);
}

void AHXPatternView::paintBelow(QPainter* painter, int height, int currentRow)
{
    //current row marker
    QColor colorBase = QColor(140, 134, 156);
    QColor colorHilite = QColor(255, 255, 255);
    QColor colorShadow = QColor(82, 89, 99);
    QPen pen(colorBase);

    painter->fillRect(0, 0, 13, (height), colorBase);

    pen.setWidth(1);
    painter->setPen(pen);
    painter->fillRect((14), (height / 2) - 8, 305, 7, m_colorCurrentRowBackground);
    painter->drawLine((14), (height / 2) - 9, (319), ((height / 2) - 9));
    painter->drawLine((14), (height / 2) - 1, (319), ((height / 2) - 1));

    //left border
    pen.setColor(colorHilite);
    painter->setPen(pen);
    painter->drawLine((13), 0, (13), height - 3);
    pen.setColor(colorShadow);
    painter->setPen(pen);
    painter->drawLine((14), 0, (14), height - 1);
    painter->setPen(pen);
}

AHXPatternView::~AHXPatternView()
{
}
