#include "OctaMED44ChanPatternView.h"
#include "visualizers/tracker.h"

OctaMED44ChanPatternView::OctaMED44ChanPatternView(Tracker *parent, unsigned int channels, int scale)
                            :MEDPatternView(parent,channels,scale)
{
    effectPad = true;
    m_emptyEffect = "00";
    m_SeparatorChannel="";
    m_xOffsetRow=0;
    m_RowLength = 40;

    m_xChannelStart = 65;
    m_channelWidth = 142;
    m_channelLastWidth = 143;
    m_channelxSpace = 2;

    m_bitmapFont3 = BitmapFont("OctaMED Pro 4");

    m_topHeight=28;
    m_bottomFrameHeight=20;
}

void OctaMED44ChanPatternView::paintAbove(QPainter* painter, int height, int currentRow)
{
    QColor colorPink(255,170,173);
    QColor colorRed(255,0,0);


    QPen pen(colorPink);
    painter->setPen(pen);

    //vumeter bottom and channel numbers
    Tracker* t = (Tracker*)this->parent();
    for(unsigned int i = 0;i<m_channels;i++)
    {
        painter->fillRect((152)+144*i,(height)-(22),4,2,QColor(170,170,170));
        painter->fillRect((156)+144*i,(height)-(22),16,2,QColor(0,136,0));
        painter->fillRect((172)+144*i,(height)-(22),4,2,QColor(187,187,187));
        t->drawText(QString::number(i),painter,(120+(i*144)),44);
    }
    t->drawText("4",painter,56,60);
    //channel separators
    for(unsigned int chan = 0;chan<m_channels-1;chan++)
    {
        painter->fillRect((207+chan*144),28,2,height-(2.0),colorPink);
        painter->fillRect((207+chan*144),(height/2)-17,2,18,colorRed);
    }

    //bottom
    QColor colorBase(153,153,170);
    QColor colorHilite(204,204,204);
    QColor colorShadow(102,102,119);
    painter->fillRect(0,height-(20),640,20,colorBase);
    painter->fillRect(178,height-(18),428,16,QColor(0,0,0));
    painter->fillRect(0,height-(20),2,20,colorShadow);
    painter->fillRect(0,height-(2),640,2,colorShadow);
    painter->fillRect(1,height-(20),639,2,colorHilite);

    //edge
    painter->fillRect(174,height-(18),1,16,colorHilite);
    painter->fillRect(175,height-(18),1,18,colorHilite);
    painter->fillRect(176,height-(20),1,18,colorShadow);
    painter->fillRect(177,height-(18),1,18,colorShadow);
    //edge
    painter->fillRect(402,height-(18),1,16,colorHilite);
    painter->fillRect(403,height-(18),1,18,colorHilite);
    painter->fillRect(404,height-(20),1,18,colorShadow);
    painter->fillRect(405,height-(18),1,18,colorShadow);
    //edge
    painter->fillRect(606,height-(18),1,16,colorHilite);
    painter->fillRect(607,height-(18),1,18,colorHilite);
    painter->fillRect(608,height-(20),1,18,colorShadow);
    painter->fillRect(609,height-(18),1,18,colorShadow);
    //edge
    painter->fillRect(638,height-(18),1,16,colorHilite);
    painter->fillRect(639,height-(18),1,18,colorHilite);


    painter->setPen(colorHilite);

    t->drawText(QString(t->m_info->title.c_str()).left(25),painter,406,height-2);

}
void OctaMED44ChanPatternView::paintBelow(QPainter* painter, int height, int currentRow)
{
    painter->fillRect(0,(height/2)-17,640,18,m_colorCurrentRowBackground);
}
QString OctaMED44ChanPatternView::effect(BaseRow* row)
{

    //*kolla octamedkällkod i libxmp om jag fattar effekterna
    //*man kan välja 16 kanaler i GUI, kolla igenom alla mmd1 och se antal kanaler
    //*vumeters visas ej i mer än 4 kanaler
    //*instrument utan not visas ej (och kan då inte heller visa lodrät linje) kolla hur det är i protracker, annars föreslå til libxmp

    if(row->effect<9 || row->effect==0xC || row->effect==0xB) return AbstractPatternView::effect(row);

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

QString OctaMED44ChanPatternView::parameter(BaseRow* row)
{
    int parameter = row->param;
    if(row->effect==0xE)
    {
        parameter=parameter & 0x0f;
        if(parameter==0) return m_emptyParameter;
        int base = parameterHex ? 16 : 10;

        QString parameterStr = QString::number(parameter,base).toUpper();
        parameterStr = parameterPad && parameterStr.length()==1 ? "0" + parameterStr : parameterStr;
        return parameterStr;
    }
    return AbstractPatternView::parameter(row);
}
OctaMED44ChanPatternView::~OctaMED44ChanPatternView()
{

}
