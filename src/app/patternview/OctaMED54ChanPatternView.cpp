#include "OctaMED54ChanPatternView.h"
#include "visualizers/tracker.h"

OctaMED54ChanPatternView::OctaMED54ChanPatternView(Tracker* parent, unsigned int channels, int scale)
    : MEDPatternView(parent, channels, scale)
{
    octaveOffset = 12;
    effectPad = true;
    m_emptyEffect = "00";
    m_SeparatorChannel = "";
    m_xOffsetRow = 0;
    //m_ColorEffect = QColor(255,0,0);
    m_colorDefault = m_ColorRowNumber = m_ColorInstrument = m_ColorEffect = m_ColorParameter = m_ColorVolume =
        m_colorEmpty = QColor(0, 0, 0);
    m_colorCurrentRowBackground = QColor(255, 255, 255);
    m_colorCurrentRowForeground = QColor(82, 121, 165);

    m_xChannelStart = 56;
    m_channelFirstWidth = 151;
    m_channelWidth = 142;
    m_channelLastWidth = 143;
    m_channelxSpace = 2;
    m_colorBackground = QColor(156, 154, 156, 180);

    m_bitmapFont3 = BitmapFont("OctaMED Pro 4");
    m_topHeight = 52;
}

QString OctaMED54ChanPatternView::effect(BaseRow* row)
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

void OctaMED54ChanPatternView::paintAbove(QPainter* painter, int height, int currentRow)
{
    QColor colorWhite(255, 255, 255);
    QColor colorGrey(156, 154, 156);

    QFont font = QFont("OctaMED Pro 4");
    font.setPixelSize(16);
    QPen pen(colorWhite);
    painter->setPen(pen);
    painter->setFont(font);
    Tracker* t = (Tracker*)this->parent();
    t->setFont(infoFont());
    //vumeter bottom and channel numbers
    for (unsigned int i = 0; i < m_channels; i++)
    {
        painter->fillRect((152) + 144 * i, (height) - (8), 4, 2, QColor(170, 170, 170));
        painter->fillRect((156) + 144 * i, (height) - (8), 16, 2, QColor(0, 136, 0));
        painter->fillRect((172) + 144 * i, (height) - (8), 4, 2, QColor(187, 187, 187));
        t->drawText(QString::number(i), painter, (120 + (i * 144)), 70);
    }
    painter->fillRect(0, height - 6, 640, 6, QColor(0, 0, 0));
    painter->fillRect(0, height - 6, 639, 2, colorWhite);
    painter->fillRect(0, height - 4, 2, 4, colorWhite);
    painter->fillRect(2, height - 4, 636, 2, colorGrey);
    painter->fillRect(1, height - 2, 1, 2, QColor(0, 0, 0));


    t->drawText("4", painter, 56, 86);
    //channel separators
    for (unsigned int chan = 0; chan < m_channels - 1; chan++)
    {
        painter->fillRect((207 + chan * 144), 54, 2, height - 58, colorWhite);
        painter->fillRect((207 + chan * 144), (height / 2) - 17, 2, 18, colorGrey);
    }
}

void OctaMED54ChanPatternView::paintBelow(QPainter* painter, int height, int currentRow)
{
    QColor colorGrey = QColor(156, 154, 156);
    //background
    painter->fillRect(0, 0, 640, height, colorGrey);
    //current row
    painter->fillRect(0, (height / 2) - 17, 640, 18, m_colorCurrentRowBackground);
}

OctaMED54ChanPatternView::~OctaMED54ChanPatternView()
{
}
