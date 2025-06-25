#include "OctaMED5ChanPatternView.h"
#include "visualizers/tracker.h"
#include "various.h"

OctaMED5ChanPatternView::OctaMED5ChanPatternView(Tracker* parent, unsigned int channels)
    : MEDPatternView(parent, channels)
{
    octaveOffset = 12;
    m_font = QFont("OctaMED Pro 4");
    m_font.setPixelSize(16);
    m_fontWidth = 8;

    m_bitmapFont = BitmapFont("OctaMED Pro 4");
    m_fontWidth = 8;
    m_fontHeight = 16;

    m_bitmapFont3 = BitmapFont("OctaMED Pro 4");

    effectPad = true;
    m_emptyEffect = "00";
    m_SeparatorChannel = "";
    m_xOffsetRow = 0;
    m_SeparatorRowNumber = " ";
    m_colorDefault = m_ColorRowNumber = m_ColorInstrument = m_ColorEffect = m_ColorParameter = m_ColorVolume =
        m_colorEmpty = QColor(0, 0, 0);
    m_colorCurrentRowBackground = QColor(255, 255, 255);
    m_colorCurrentRowForeground = QColor(82, 121, 165);
    m_RowLength = 80;

    m_xChannelStart = 32;
    m_channelWidth = 71;
    m_channelLastWidth = 104;
    m_channelxSpace = 1;
    m_colorBackground = QColor(156, 154, 156, 180);
    m_topHeight = 52;
    m_bottomFrameHeight = 3;

}

QString OctaMED5ChanPatternView::effect(BaseRow* row)
{
    //*kolla octamedk�llkod i libxmp om jag fattar effekterna
    //*man kan v�lja 16 kanaler i GUI, kolla igenom alla mmd1 och se antal kanaler
    //*vumeters visas ej i mer �n 4 kanaler
    //*instrument utan not visas ej (och kan d� inte heller visa lodr�t linje) kolla hur det �r i protracker, annars f�resl� til libxmp

    if (row->effect < 9 || row->effect == 0xC || row->effect == 0xB) return AbstractPatternView::effect(row);

    if (row->effect == 0x9) return "19";
    if (row->effect == 0xA) return "0D";
    if (row->effect == 0xD) return "0A"; //sometimes 0D = 0F...?
    if (row->effect == 0xE) return "16"; //sometimes 0E = 0F...?
    if (row->effect == 0xF) return "09";
    if (row->effect == 0x86) return "14";
    if (row->effect == 0xA6) return "15";
    if (row->effect == 0xAB) return "0F";
    if (row->effect == 0xAD) return "1A";
    if (row->effect == 0xAE) return "1B";
    if (row->effect == 0xAF) return "11";
    if (row->effect == 0xB0) return "12";
    if (row->effect == 0xB3) return "1E";

    return "#" + AbstractPatternView::effect(row) + "#";
}

void OctaMED5ChanPatternView::paintAbove(QPainter* painter, int height, int currentRow)
{
    QColor colorWhite(255, 255, 255);
    QColor colorGrey(156, 154, 156);

    QFont font = QFont("OctaMED Pro 4");
    font.setPixelSize(16);
    QPen pen(colorWhite);
    painter->setPen(pen);
    painter->setFont(font);

    //channel numbers
    for (int i = 0; i < 8; i++)
    {
        drawText(QString::number(i), painter, (56 + (i * 72)), 70,infoFont());
    }
    drawText(QString::number(m_channels), painter, 24, 86,infoFont());

    //channel separators
    for (int chan = 0; chan < 7; chan++)
    {
        painter->fillRect((103 + chan * 72), 54, 1, height-54, colorWhite);
        painter->fillRect((103 + chan * 72), height-20, 1, 18, colorGrey);
    }
    painter->fillRect(0, height - 6, 640, 6, QColor(0, 0, 0));
    painter->fillRect(0, height - 6, 639, 2, colorWhite);
    painter->fillRect(0, height - 4, 2, 4, colorWhite);
    painter->fillRect(2, height - 4, 636, 2, colorGrey);
    painter->fillRect(1, height - 2, 1, 2, QColor(0, 0, 0));
}

void OctaMED5ChanPatternView::paintBelow(QPainter* painter, int height, int currentRow)
{
    QColor colorGrey = QColor(156, 154, 156);
    //background
    painter->fillRect(0, 0, 640, height, colorGrey);
    //current row
    painter->fillRect(0, (height / 2) - 18, 640, 18, m_colorCurrentRowBackground);
}

void::OctaMED5ChanPatternView::paintTop(QPainter* painter,Info* info, unsigned int m_currentPattern, unsigned int m_currentPosition, unsigned int m_currentSpeed, unsigned int m_currentBPM, unsigned int m_currentRow)
{
    m_topHeight = 54;
    QColor colorBase(156, 154, 156);
    QColor colorHilite(255, 255, 255);
    QColor colorShadow(0, 0, 0);
;

    int left = 0;

    //top, songname


    //background
    painter->fillRect(left, 0, 640, m_topHeight, colorBase);
    painter->fillRect(left, 0, 640, 20, colorHilite);
    painter->fillRect(left, 20, 640, 4, colorShadow);
    painter->fillRect(left, 50, 640, 4, colorShadow);

    //right
    painter->fillRect(left + 639, 2, 1, 18, colorShadow);

    //buttons
    painter->fillRect(left, 22, 2, 26, colorHilite);
    painter->fillRect(left + 71, 22, 4, 26, colorHilite);
    painter->fillRect(left + 160, 22, 4, 26, colorHilite);
    painter->fillRect(left + 249, 22, 4, 26, colorHilite);
    painter->fillRect(left + 634, 22, 2, 26, colorHilite);
    painter->fillRect(left + 71, 22, 1, 2, colorShadow);
    painter->fillRect(left + 160, 22, 1, 2, colorShadow);
    painter->fillRect(left + 249, 22, 1, 2, colorShadow);
    painter->fillRect(left + 634, 22, 1, 2, colorShadow);
    painter->fillRect(left + 22, 22, 4, 26, colorShadow);
    painter->fillRect(left + 95, 22, 4, 26, colorShadow);
    painter->fillRect(left + 184, 22, 4, 26, colorShadow);
    painter->fillRect(left + 265, 22, 4, 26, colorShadow);
    painter->fillRect(left + 636, 22, 4, 2, colorBase);
    painter->fillRect(left, 22, 23, 2, colorHilite);
    painter->fillRect(left + 72, 22, 24, 2, colorHilite);
    painter->fillRect(left + 161, 22, 24, 2, colorHilite);
    painter->fillRect(left + 250, 22, 16, 2, colorHilite);
    painter->fillRect(left + 1, 46, 24, 2, colorShadow);
    painter->fillRect(left + 1, 46, 24, 2, colorShadow);
    painter->fillRect(left + 74, 46, 24, 2, colorShadow);
    painter->fillRect(left + 163, 46, 24, 2, colorShadow);
    painter->fillRect(left + 252, 46, 16, 2, colorShadow);
    painter->fillRect(left + 252, 46, 16, 2, colorShadow);

    painter->fillRect(left + 25, 46, 49, 2, colorHilite);
    painter->fillRect(left + 25, 46, 49, 2, colorHilite);
    painter->fillRect(left + 98, 46, 65, 2, colorHilite);
    painter->fillRect(left + 187, 46, 65, 2, colorHilite);
    painter->fillRect(left + 268, 46, 368, 2, colorHilite);
    painter->fillRect(left, 50, 639, 2, colorHilite);
    painter->fillRect(left, 52, 1, 2, colorHilite);

    painter->setPen(QColor(0, 0, 0));
    drawText("Sg", painter, left + (4), 44, infoFont());
    drawText("Sc", painter, left + (77), 44, infoFont());
    drawText("Sq", painter, left + (166), 44, infoFont());
    drawText(
            QString("%1").arg(m_currentPosition + 1, 3, 10, QChar('0')) + "/" + QString("%1").arg(
                    info->numOrders, 3, 10, QChar('0')), painter, left + (191), 44,infoFont());
    drawText("B", painter, left + (255), 44, infoFont());
    drawText(
            QString("%1").arg(m_currentPattern, 3, 10, QChar('0')) + "/" + QString("%1").arg(
                    info->numPatterns - 1, 3, 10, QChar('0')) + ":", painter, left + (272), 44,infoFont());

    //underline
    painter->fillRect(left + (12), 42, 1, 2, colorShadow);
    painter->fillRect(left + (19), 42, 1, 2, colorShadow);
    painter->fillRect(left + (85), 42, 8, 2, colorShadow);
    painter->fillRect(left + (174), 42, 4, 2, colorShadow);
    painter->fillRect(left + (255), 42, 8, 2, colorShadow);

    drawText("Song: " + fromUtf8OrLatin1(info->title), painter, left + (5), 18, infoFont());
}
OctaMED5ChanPatternView::~OctaMED5ChanPatternView()
{
}
