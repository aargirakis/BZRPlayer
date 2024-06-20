#include "OctaMEDSoundstudioPatternView.h"
#include "visualizers/tracker.h"

OctaMEDSoundstudioPatternView::OctaMEDSoundstudioPatternView(Tracker *parent, unsigned int channels, int scale)
                            :AbstractPatternView(parent,channels,scale)
{
    octaveOffset = -12;
    m_font = QFont("AHX Thin");
    m_font.setPixelSize(8);
    m_fontWidth = 6;

    m_bitmapFont = BitmapFont("AHX Thin");
    m_fontWidth = 6;
    m_fontHeight = 8;

    m_bitmapFont2 = BitmapFont("OctaMED Soundstudio");

    rowNumberOffset = 0;
    m_emptyInstrument="0";
    instrumentPad = false;
    effectPad = true;
    m_emptyEffect = "00";
    m_colorCurrentRowBackground = QColor(255,255,255);
    m_colorCurrentRowForeground = QColor(82,121,165);
    m_colorDefault = m_ColorRowNumber = m_ColorInstrument  = m_ColorEffect = m_ColorParameter = m_ColorVolume = m_colorEmpty = QColor(0,0,0);
    m_xOffsetRow=4;

    m_SeparatorRowNumber = m_SeparatorChannel = " ";
    int minChannels = m_channels<11 ? 11: m_channels;
    m_RowLength = 4+(minChannels*10);
    m_xChannelStart = 26;
    m_channelWidth = 58;
    m_channelxSpace = 2;
    m_colorBackground = QColor(156,154,156,180);
    m_topHeight=32;
    m_bottomFrameHeight=34;
}
BitmapFont OctaMEDSoundstudioPatternView::infoFont()
{
    return m_bitmapFont2;
}
QString OctaMEDSoundstudioPatternView::rowNumber(int rowNumber)
{
    int base = rowNumberHex ? 16 : 10;

    QString rowNumberStr = QString::number(rowNumber+rowNumberOffset,base).toUpper();
    rowNumberStr = rowNumberStr.length()==1 ? "00" + rowNumberStr : rowNumberStr;
    rowNumberStr = rowNumberStr.length()==2 ? "0" + rowNumberStr : rowNumberStr;
    return rowNumberStr;
}
QString OctaMEDSoundstudioPatternView::parameter(BaseRow* row)
{
    if(row->effect==0x8)
    {
        if(row->param==0x80)
        {
            return  "00";
        }
    }
    return AbstractPatternView::parameter(row);
}

QString OctaMEDSoundstudioPatternView::effect(BaseRow* row)
{

    //*kolla octamedkällkod i libxmp om jag fattar effekterna
    //*man kan välja 16 kanaler i GUI, kolla igenom alla mmd1 och se antal kanaler
    //*vumeters visas ej i mer än 4 kanaler
    //*instrument utan not visas ej (och kan då inte heller visa lodrät linje) kolla hur det är i protracker, annars föreslå til libxmp

    if(row->effect<8 || row->effect==0xC || row->effect==0xB) return AbstractPatternView::effect(row);

    if(row->effect==0x8)
    {
        if(row->param==0x80)
        {
                return  "2E";
        }
        else
        {
            return "08";
        }
    }
    if(row->effect==0x9) return  "19";
    if(row->effect==0xA) return  "0D";
    if(row->effect==0xD) return  "0A"; //sometimes 0D = 0F...?
    if(row->effect==0xE) return  "16"; //sometimes 0E = 0F...?
    if(row->effect==0xF) return  "09";
    if(row->effect==0x86) return "14";
    if(row->effect==0xAB) return "0F";
    if(row->effect==0xAD) return "1A";
    if(row->effect==0xAE) return "1B";
    if(row->effect==0xAF) return "11";
    if(row->effect==0xB0) return "12";
    if(row->effect==0xB3) return "1E";

    return "#" + AbstractPatternView::effect(row) + "#";
}
QString OctaMEDSoundstudioPatternView::instrument(BaseRow* row)
{
    int instrument = row->sample;
    QString padding = m_SeparatorRowNumber;
    if(instrument>=32)
    {
        padding="";
    }
    return padding+QString::number(instrument,32).toUpper();
}
void OctaMEDSoundstudioPatternView::paintAbove(QPainter* painter, int height, int currentRow)
{
    QColor colorBase(156,154,156);
    QColor colorHilite(255,255,255);
    QColor colorShadow(0,0,0);

    int minChannels = m_channels<11 ? 11: m_channels;
    //background
    painter->fillRect(0,height-(34),(25+(minChannels*60)),33,colorBase);
    //left
    painter->fillRect(0,height-(33),1,34,colorHilite);
    //right
    painter->fillRect(0+(24+(minChannels*60)),height-34,1,34,colorShadow);
    //top
    painter->fillRect(0,height-(34),(24+(minChannels*60)),1,colorHilite);
    //bottom
    painter->fillRect(1,height,(25+(minChannels*60)),1,colorShadow);

    painter->fillRect(1,height-(19),(25+(minChannels*60)),1,colorShadow);
    painter->fillRect(3,height-(18),1,18,colorShadow);
    painter->fillRect((21+(minChannels*60)),height-(18),1,18,colorHilite);
    painter->fillRect(4,height-(1),(17+(minChannels*60)),1,colorHilite);

    QFont font("AHX Thin");
    font.setPixelSize(8);
    QFont fontBig("OctaMED Soundstudio");
    fontBig.setPixelSize(13);
    painter->setFont(fontBig);
    painter->setPen(QColor(0,0,0));
    Tracker* t = (Tracker*)this->parent();
    t->setFont(infoFont());
    t->drawText(QString("Information - BPM ") + QString::number(t->m_currentBPM) + "/" + QString::number(t->m_currentSpeed),painter,30,height-(20));

    //buttons
    for(int i = 0;i<3;i++)
    {
        painter->fillRect(6+((i+1)*112),height-(16),4,13,colorHilite);
        if(i<2)
        {
            painter->fillRect(43+((i+1)*112),height-(16),4,13,colorShadow);
        }
        painter->fillRect(8+(i*112),height-(16),36,1,colorHilite);
        painter->fillRect(44+(i*112),height-(16),75,1,colorShadow);
        painter->fillRect(9+(i*112),height-(4),37,1,colorShadow);
        painter->fillRect(46+(i*112),height-(4),75,1,colorHilite);
    }

    //first button
    painter->fillRect(8,height-(16),1,13,colorHilite);
    painter->fillRect(9,height-(16),1,12,colorHilite);
    painter->fillRect(43,height-(15),4,11,colorShadow);

    //last button
    painter->fillRect(369,height-(16),4,13,colorShadow);
    painter->fillRect(343,height-(16),27,1,colorHilite);
    painter->fillRect(345,height-(4),27,1,colorShadow);
    painter->fillRect(630,height-(16),2,13,colorHilite);
    painter->fillRect(372,height-(4),258,1,colorHilite);
    painter->fillRect(373,height-(16),258,1,colorShadow);

    painter->setFont(font);
    t->setFont(bitmapFont());
    t->drawText("Sg",painter,21,height-(5));
    t->drawText("Sc",painter,133,height-(5));
    t->drawText("Sq",painter,245,height-(5));
    t->drawText(QString::number(t->m_currentPosition+1) + "/" + QString::number(t->m_info->numOrders),painter,290,height-(5));
    t->drawText("B",painter,355,height-(5));
    t->drawText(QString::number(t->m_currentPattern) + "/" + QString::number(t->m_info->numPatterns-1)+": ",painter,376,height-(5));

    //underline
    painter->fillRect(32,height-(6),1,1,colorShadow);
    painter->fillRect(139,height-(6),6,1,colorShadow);
    painter->fillRect(251,height-(6),3,1,colorShadow);
    painter->fillRect(355,height-(6),6,1,colorShadow);

}

void OctaMEDSoundstudioPatternView::paintBelow(QPainter* painter, int height, int currentRow)
{
    int minChannels = m_channels<11 ? 11: m_channels;
    QColor colorGrey = QColor(156,154,156);
    //background
    painter->fillRect(0,0,(25+(minChannels*60)),height,colorGrey);
    painter->fillRect(0,(height/2)-9,(25+(minChannels*60)),9,m_colorCurrentRowBackground);

    //left bar
    painter->fillRect(0,0,1,height,QColor(255,255,255));
    painter->fillRect(3,0,1,height,QColor(0,0,0));

    //channel separators
    painter->fillRect(25,0,2,height,QColor(255,255,255));

    for(int chan = 1;chan<minChannels;chan++)
    {
        painter->fillRect((24+chan*60),0,2,height,QColor(255,255,255));
    }

}
OctaMEDSoundstudioPatternView::~OctaMEDSoundstudioPatternView()
{

}
