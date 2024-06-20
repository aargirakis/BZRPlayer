#include "StarTrekker13PatternView.h"

StarTrekker13PatternView::StarTrekker13PatternView(Tracker *parent, unsigned int channels, int scale)
                            :AbstractPatternView(parent,channels,scale)
{
    rowNumberOffset=0;
    octaveOffset = 48;
    m_font2 = QFont("Startrekker 1.3 Double Height");
    m_font2.setPixelSize(14);
    m_font = QFont("Startrekker 1.3");
    m_font.setPixelSize(7);
    m_fontWidth = 8;

    m_bitmapFont = BitmapFont("Startrekker 1.3");
    m_bitmapFont2 = BitmapFont("Startrekker 1.3 Double Height");
    m_fontWidth = 8;
    m_fontHeight = 7;

    m_font.setStyleStrategy(QFont::NoAntialias);
    m_colorCurrentRowBackground = QColor(189,138,99);
    m_colorDefault = m_ColorRowNumber = m_ColorInstrument = m_ColorEffect = m_ColorParameter = m_ColorEffect2 = m_ColorParameter2 = m_ColorVolume = m_colorEmpty = QColor(156,170,33);

    m_fontWidth = 8;
    m_font.setStyleStrategy(QFont::NoAntialias);
    m_RowEnd = m_SeparatorRowNumber = m_SeparatorChannel = "'";
    m_RowLength = 40;
    m_xOffsetRow=8;
    m_xChannelStart = 29;
    m_channelWidth = 70;
    m_channelLastWidth = 72;
    m_channelxSpace = 2;

    m_ibuttonPrevSampleWidth=11;
    m_ibuttonPrevSampleHeight=11;
    m_ibuttonPrevSampleX=11;
    m_ibuttonPrevSampleY=33;
    m_ibuttonNextSampleWidth=11;
    m_ibuttonNextSampleHeight=11;
    m_ibuttonNextSampleX=0;
    m_ibuttonNextSampleY=33;
    m_topHeight=44;
}
QFont StarTrekker13PatternView::currentRowFont()
{
    return m_font2;
}

BitmapFont StarTrekker13PatternView::currentRowBitmapFont()
{
    return m_bitmapFont2;
}

StarTrekker13PatternView::~StarTrekker13PatternView()
{

}
void StarTrekker13PatternView::paintAbove(QPainter* painter, int height, int currentRow)
{

    QColor colorBase(189,138,99);
    QColor colorHilite(222,186,156);
    QColor colorShadow(140,85,49);

    for(unsigned int chan = 0;chan<m_channels;chan++)
    {
        //current row per channel

        int extra=0;
        if(chan==m_channels-1)
        {
            extra=2;
        }
        //bottom hilight
        painter->fillRect((29)+chan*72,(height)-3,1,1,colorShadow);
        painter->fillRect((30)+chan*72,(height)-3,(70+extra),1,colorHilite);

    }

    painter->fillRect((3),(height)-3,25,1,colorHilite);
    painter->fillRect((1),(height)-2,318,1,colorBase);
    painter->fillRect(1,(height)-1,319,1,colorShadow);

}

void StarTrekker13PatternView::paintBelow(QPainter* painter, int height, int currentRow)
{
    int left=3;

    QColor colorBase(189,138,99);
    QColor colorHilite(222,186,156);
    QColor colorShadow(140,85,49);

    QPen pen(colorBase);
    pen.setWidth(1);
    painter->setPen(pen);
    int topOffset=-4;


    //left border
    pen.setColor(colorHilite);
    painter->setPen(pen);
    painter->drawLine(((left-2)),45,((left-2)),height);
    pen.setColor(colorBase);
    painter->setPen(pen);
    painter->drawLine(((left-1)),45,((left-1)),height);
    pen.setColor(colorShadow);
    painter->setPen(pen);
    painter->drawLine(((left)),45,((left)),height);


    for(unsigned int chan = 0;chan<m_channels;chan++)
    {
        //channel dividers
        pen.setColor(colorHilite);
        painter->setPen(pen);
        painter->drawLine((left+25+chan*72),45,(left+25+chan*72),height);
        pen.setColor(colorBase);
        painter->setPen(pen);
        painter->drawLine((left+26+chan*72),45,(left+26+chan*72),height);
        pen.setColor(colorShadow);
        painter->setPen(pen);
        painter->drawLine((left+27+chan*72),45,(left+27+chan*72),height);

        //current row per channel

        int extra=0;
        if(chan==m_channels-1)
        {
            extra=2;
        }
        //top hilite
        painter->fillRect(((left+26))+chan*72,(height/2)-2+(topOffset),(70+extra),2,colorHilite);
        //bottom shadow
        painter->fillRect(((left+26))+chan*72,(height/2)+10+(topOffset),(70+extra),2,colorShadow);
    }

    //right border
    pen.setColor(colorHilite);
    painter->setPen(pen);
    painter->drawLine(318,45,318,height);
    pen.setColor(colorBase);
    painter->setPen(pen);
    painter->drawLine(319,45,319,height);
    pen.setColor(colorShadow);
    painter->setPen(pen);
    painter->drawLine(320,45,320,height);


    topOffset=-2;
    painter->fillRect(((left-2)),(height/2)-2+(topOffset),317,10,colorBase);

    //bottom shadow
    topOffset=-4;
    painter->fillRect((2),(height/2)+10+(topOffset),26,2,colorShadow);


    //leftmost current row

    topOffset=-4;
    painter->fillRect(((left)),(height/2)-2+(topOffset),24,2,colorHilite);
    //leftmost current ror antialias
    painter->fillRect(((left-1)),(height/2)-2+(topOffset),1,2,colorBase);

}
