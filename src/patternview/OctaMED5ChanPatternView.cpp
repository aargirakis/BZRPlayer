#include "OctaMED5ChanPatternView.h"
#include "visualizers/tracker.h"

OctaMED5ChanPatternView::OctaMED5ChanPatternView(Tracker *parent, unsigned int channels, int scale)
                            :MEDPatternView(parent,channels,scale)
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
    m_SeparatorChannel="";
    m_xOffsetRow=0;
    m_SeparatorRowNumber=" ";
    m_colorDefault = m_ColorRowNumber = m_ColorInstrument  = m_ColorEffect = m_ColorParameter = m_ColorVolume = m_colorEmpty = QColor(0,0,0);
    m_colorCurrentRowBackground = QColor(255,255,255);
    m_colorCurrentRowForeground = QColor(82,121,165);
    m_RowLength = 80;

    m_xChannelStart = 32;
    m_channelWidth = 71;
    m_channelLastWidth = 104;
    m_channelxSpace = 1;
    m_colorBackground = QColor(156,154,156,180);
    m_topHeight=52;
    m_bottomFrameHeight=3;
}
QString OctaMED5ChanPatternView::effect(BaseRow* row)
{

    //*kolla octamedk�llkod i libxmp om jag fattar effekterna
    //*man kan v�lja 16 kanaler i GUI, kolla igenom alla mmd1 och se antal kanaler
    //*vumeters visas ej i mer �n 4 kanaler
    //*instrument utan not visas ej (och kan d� inte heller visa lodr�t linje) kolla hur det �r i protracker, annars f�resl� til libxmp

    if(row->effect<9 || row->effect==0xC || row->effect==0xB) return AbstractPatternView::effect(row);

    if(row->effect==0x9) return  "19";
    if(row->effect==0xA) return  "0D";
    if(row->effect==0xD) return  "0A"; //sometimes 0D = 0F...?
    if(row->effect==0xE) return  "16"; //sometimes 0E = 0F...?
    if(row->effect==0xF) return  "09";
    if(row->effect==0x86) return "14";
    if(row->effect==0xA6) return "15";
    if(row->effect==0xAB) return "0F";
    if(row->effect==0xAD) return "1A";
    if(row->effect==0xAE) return "1B";
    if(row->effect==0xAF) return "11";
    if(row->effect==0xB0) return "12";
    if(row->effect==0xB3) return "1E";

    return "#" + AbstractPatternView::effect(row) + "#";
}
void OctaMED5ChanPatternView::paintAbove(QPainter* painter, int height, int currentRow)
{
    QColor colorWhite(255,255,255);
    QColor colorGrey(156,154,156);

    QFont font = QFont("OctaMED Pro 4");
    font.setPixelSize(16);
    QPen pen(colorWhite);
    painter->setPen(pen);
    painter->setFont(font);
    Tracker* t = (Tracker*)this->parent();
    t->setFont(infoFont());

    //channel numbers
    for(int i = 0;i<8;i++)
    {

        t->drawText(QString::number(i),painter,(56+(i*72)),70);
    }
    t->drawText(QString::number(m_channels),painter,24,86);

    //channel separators
    for(int chan = 0;chan<7;chan++)
    {
        painter->fillRect((103+chan*72),54,1,height,colorWhite);
        painter->fillRect((103+chan*72),(height/2)-18,1,18,colorGrey);
    }
    painter->fillRect(0,height-6,640,6,QColor(0,0,0));
    painter->fillRect(0,height-6,639,2,colorWhite);
    painter->fillRect(0,height-4,2,4,colorWhite);
    painter->fillRect(2,height-4,636,2,colorGrey);
    painter->fillRect(1,height-2,1,2,QColor(0,0,0));
}
void OctaMED5ChanPatternView::paintBelow(QPainter* painter, int height, int currentRow)
{

    QColor colorGrey = QColor(156,154,156);
    //background
    painter->fillRect(0,0,640,height,colorGrey);
    //current row
    painter->fillRect(0,(height/2)-18,640,18,m_colorCurrentRowBackground);
}

OctaMED5ChanPatternView::~OctaMED5ChanPatternView()
{

}
