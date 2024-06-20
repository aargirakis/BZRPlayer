#include "IceTrackerPatternView.h"

IceTrackerPatternView::IceTrackerPatternView(Tracker *parent, unsigned int channels, int scale)
                            :AbstractPatternView(parent,channels,scale)
{
    rowNumberOffset=0;
    octaveOffset = 48;
    m_font2 = QFont("Protracker 1.0 Double Height");
    m_font2.setPixelSize(14);
    m_font = QFont("Ice Tracker");
    m_font.setPixelSize(7);
    m_fontWidth = 8;
    m_font.setStyleStrategy(QFont::NoAntialias);

    m_bitmapFont = BitmapFont("Ice Tracker");
    m_bitmapFont2 = BitmapFont("Protracker 1.0 Double Height");
    m_fontWidth = 8;
    m_fontHeight = 7;


    m_ColorRowNumberBackground = m_ColorWindowBackground = "738a9c";

    m_colorDefault = m_ColorRowNumber = m_ColorInstrument = m_ColorEffect = m_ColorParameter = m_ColorEffect2 = m_ColorParameter2 = m_ColorVolume = m_colorEmpty = QColor(66,85,99);
    m_colorCurrentRowBackground = QColor(115,138,156);
    m_RowEnd = m_SeparatorRowNumber = m_SeparatorChannel = " ";
    m_RowLength = 40;
    m_xOffsetRow=8;
    m_channelLastWidth = 71;

    m_colorBackground = QColor(115,138,156,180);

    m_ibuttonPrevSampleWidth=11;
    m_ibuttonPrevSampleHeight=11;
    m_ibuttonPrevSampleX=11;
    m_ibuttonPrevSampleY=22;
    m_ibuttonNextSampleWidth=11;
    m_ibuttonNextSampleHeight=11;
    m_ibuttonNextSampleX=0;
    m_ibuttonNextSampleY=22;
}
QFont IceTrackerPatternView::currentRowFont()
{
    return m_font2;
}
BitmapFont IceTrackerPatternView::currentRowBitmapFont()
{
    return m_bitmapFont2;
}
void IceTrackerPatternView::paintAbove(QPainter* painter, int height, int currentRow)
{

    QColor colorBase(115,138,156);
    QColor colorHilite(173,186,206);
    QColor colorShadow(66,85,115);


    for(unsigned int chan = 0;chan<m_channels;chan++)
    {
        int extra=0;
        if(chan==m_channels-1)
        {
            extra=2;
        }
        painter->fillRect((30)+chan*72,(height)-3,(70+extra),1,colorHilite);
        painter->fillRect((29)+chan*72,32,(70+extra),1,colorShadow);
    }
    painter->fillRect((5),32,22,1,colorShadow);
    painter->fillRect((6),(height)-3,22,1,colorHilite);
    painter->fillRect((1),(height)-2,318,1,colorBase);
    painter->fillRect(1,(height)-1,319,1,colorShadow);

    //scrollbar
    QPen pen(colorShadow);
    pen.setWidth(1);
    pen.setColor(colorShadow);
    painter->setPen(pen);
    painter->drawLine((2),(0.25*height),(2),(0.75*height));
    painter->drawLine((2),(0.25*height),(4),(0.25*height));
    pen.setColor(colorHilite);
    painter->setPen(pen);
    painter->drawLine((5),(0.25*height)+(1),(5),(0.75*height)+(1));
    painter->drawLine(3,(0.75*height)+(1),(4),(0.75*height)+(1));

    //current position in scrollbar
    pen.setColor(QColor(222,223,222));
    painter->setPen(pen);
    float currentRowPos = currentRow/100.0;
    int yPos = ((currentRowPos*height*0.78)+0.25*height)+(1);
    painter->drawLine((3),yPos,(4),yPos);

    //1px antialias
    painter->fillRect(11,32,1,1,colorBase);
    painter->fillRect(22,32,1,1,colorBase);
    painter->fillRect(45,32,1,1,colorBase);

    //1 pixel antialiasing
    painter->fillRect(317,32,1,1,colorBase);

    //1 pixel antialiasing
    painter->fillRect(0,32,1,1,colorHilite);



}

void IceTrackerPatternView::paintBelow(QPainter* painter, int height, int currentRow)
{
    int left=4;

    QColor colorBase(115,138,156);
    QColor colorHilite(173,186,206);
    QColor colorShadow(66,85,115);

    painter->fillRect(0,0,320,height,colorBase);


    QPen pen(colorBase);
    pen.setWidth(1);
    painter->setPen(pen);
    int topOffset=-4;


    //left border
    pen.setColor(colorHilite);
    painter->setPen(pen);
    painter->drawLine(((left-3)),0,((left-3)),height);
    pen.setColor(colorBase);
    painter->setPen(pen);
    painter->drawLine(((left-2)),0,((left-2)),height);
    pen.setColor(colorShadow);
    painter->setPen(pen);
    painter->drawLine(((left+2)),0,((left+2)),height-4);
    //1 pixel bottom antialiasing
    pen.setColor(colorBase);
    painter->setPen(pen);
    painter->drawLine(((left-1)),height-3,(left-1),height-3);
    //1 pixel bottom antialiasing
    pen.setColor(colorBase);
    painter->setPen(pen);
    painter->drawLine(((left-3)),height-1,(left-3),height-1);


    //current row left
    //top hilite
    painter->fillRect(((left+2)),(height/2)-2+(topOffset),20,2,colorHilite);
    //bottom shadow
    painter->fillRect(((left+3)),(height/2)+10+(topOffset),20,2,colorShadow);

//    //left hilite
    painter->fillRect(((left+2)),(height/2)+(topOffset),(1),10,colorHilite);

//    //right shadow
    painter->fillRect(((left+22)),(height/2)+(topOffset),(1),10,colorShadow);


    //main
    painter->fillRect(((left+4)),(height/2)-4,17,10,colorBase);


    for(unsigned int chan = 0;chan<m_channels;chan++)
    {
        //channel dividers
        pen.setColor(colorHilite);
        painter->setPen(pen);
        painter->drawLine((left+24+chan*72),0,(left+24+chan*72),height);
        pen.setColor(colorBase);
        painter->setPen(pen);
        painter->drawLine((left+25+chan*72),0,(left+25+chan*72),height);
        pen.setColor(colorShadow);
        painter->setPen(pen);
        painter->drawLine((left+26+chan*72),0,(left+26+chan*72),(height)-4);
        //1 pixel top antialiasing
        painter->fillRect((left+23+chan*72),32,1,1,colorBase);
        //1 pixel bottom antialiasing
        painter->fillRect((left+26+chan*72),(height)-2,1,1,colorBase);

        //current row per channel

        int extra=0;
        if(chan==m_channels-1)
        {
            extra=2;
        }

        //top hilite
        painter->fillRect(((left+26))+chan*72,(height/2)-2+(topOffset),(68+extra),2,colorHilite);
        //bottom shadow
        painter->fillRect(((left+27))+chan*72,(height/2)+10+(topOffset),(68+extra),2,colorShadow);

        //left hilite
        painter->fillRect(((left+26))+chan*72,(height/2)+(topOffset),(1),10,colorHilite);


        //right shadow
        painter->fillRect(((left+94+extra))+chan*72,(height/2)+(topOffset),(1),10,colorShadow);

        //main
        painter->fillRect(((left+28))+chan*72,(height/2)-4,(65+extra),10,colorBase);



    }

    //right border
    pen.setColor(colorHilite);
    painter->setPen(pen);
    painter->drawLine(318,0,317,height);
    pen.setColor(colorBase);
    painter->setPen(pen);
    painter->drawLine(319,0,318,height);
    pen.setColor(colorShadow);
    painter->setPen(pen);
    painter->drawLine(320,0,319,height);


    //1 pixel antialiasing
    painter->fillRect(0,height-1,1,1,colorBase);



    QColor colorGreenHiliteColor(0,239,0);
    QColor colorGreenBaseColor(0,170,0);
    QColor colorGreenShadowColor(0,101,0);
    //vumeters base
    for(unsigned int chan = 0;chan<m_channels;chan++)
    {
        painter->fillRect(((left+29))+chan*72+22,(height/2)-2+(topOffset),2,1,colorGreenHiliteColor);
        painter->fillRect(((left+31))+chan*72+22,(height/2)-2+(topOffset),6,1,colorGreenBaseColor);
        painter->fillRect(((left+37))+chan*72+22,(height/2)-2+(topOffset),2,1,colorGreenShadowColor);
    }
}
IceTrackerPatternView::~IceTrackerPatternView()
{

}
