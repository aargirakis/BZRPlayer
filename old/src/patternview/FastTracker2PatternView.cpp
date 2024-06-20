#include "FastTracker2PatternView.h"
#include "qdebug.h"
#include "visualizers/tracker.h"
FastTracker2PatternView::FastTracker2PatternView(Tracker *parent, unsigned int channels, int scale)
                            :AbstractPatternView(parent,channels,scale)
{

    instrumentPad = true;

    rowNumberHex = true;
    rowNumberOffset = 0;
    m_rowNumbersLastChannelEnabled = true;
    octaveOffset=12;
    volumeHex = true;
    volumeEnabled = true;
    m_emptyInstrument = "''";
    m_font2 = QFont("Fasttracker 2 small");
    m_emptyVolume = "@@";
    m_instrumentPaddingCharacter = "'";
    m_emptyEffect="0";
    m_font = QFont("Fasttracker 2");
    m_font.setPixelSize(8);
    m_fontWidth = 8;

    m_bitmapFont2 = BitmapFont("Fasttracker 2 hex");
    m_bitmapFont3 = BitmapFont("Fasttracker 2 GUI");

    m_bitmapFont = BitmapFont("Fasttracker 2");
    m_fontWidth = 8;
    m_fontHeight = 8;

    m_fontEffects = QFont("Fasttracker 2 small");
    m_fontEffects.setPixelSize(8);
    m_fontInstrument = m_fontEffects;
    m_fontParameters = QFont("Fasttracker 2 small");

    m_bitmapFontEffects = BitmapFont("Fasttracker 2 small");
    m_bitmapFontInstrument = m_bitmapFontEffects;
    m_bitmapFontParameters = m_bitmapFontEffects;

    m_fontParameters.setPixelSize(8);
    m_fontWidthInstrument = 4;
    m_fontWidthEffects = 4;
    m_fontWidthParameters = 4;
    m_fontWidthSeparatorNote = 4;
    m_SeparatorVolume="'";
    m_SeparatorInstrument="'";
    m_ColorRowNumberHighLightFrequency=4;
    m_ColorRowNumberHighLight = QColor(255,255,255);


    m_colorCurrentRowBackground = QColor(73,117,130);
    m_colorCurrentRowForeground = QColor(255,255,255);
    m_colorDefault = m_ColorRowNumber = m_ColorInstrument = m_ColorEffect = m_ColorParameter = m_ColorEffect2 = m_ColorParameter2 = m_ColorVolume = m_colorEmpty = QColor(255,255,130);

    m_SeparatorNote="'";

    m_SeparatorRowNumber = m_SeparatorChannel = m_SeparatorRowNumberLast = "'";

    m_yOffsetCurrentRowAfter=2;
    m_yOffsetCurrentRowBefore=-2;
    m_xOffsetRow=8;
    m_RowLength = 4+(m_channels*9)+3;

    m_xChannelStart = 29;
    m_channelWidth = 69;
    m_channelxSpace = 3;
    m_topHeight=32;
    m_width=640;
    m_height=480;

}
BitmapFont FastTracker2PatternView::infoFont()
{
    return m_bitmapFont2;
}
BitmapFont FastTracker2PatternView::infoFont2()
{
    return m_bitmapFont3;
}
int FastTracker2PatternView::fontWidthEffects()
{
    return m_fontWidthEffects;
}
QFont FastTracker2PatternView::fontEffects()
{
    return m_fontEffects;
}
BitmapFont FastTracker2PatternView::bitmapFontEffects()
{
    return m_bitmapFontEffects;
}
int FastTracker2PatternView::fontWidthParameters()
{
    return m_fontWidthParameters;
}
int FastTracker2PatternView::fontWidthSeparatorNote()
{
    return m_fontWidthSeparatorNote;
}
int FastTracker2PatternView::fontWidthInstrument()
{
    return m_fontWidthInstrument;
}
BitmapFont FastTracker2PatternView::bitmapFontParameters()
{
    return m_bitmapFontParameters;
}
QFont FastTracker2PatternView::fontParameters()
{
    return m_fontParameters;
}
BitmapFont FastTracker2PatternView::bitmapFontInstrument()
{
    return m_bitmapFontInstrument;
}
QFont FastTracker2PatternView::fontInstrument()
{
    return m_fontEffects;
}
const char* FastTracker2PatternView::NOTES[121] =
{
        "",
        "C-0", "C#0", "D-0", "D#0", "E-0", "F-0", "F#0", "G-0", "G#0", "A-0", "A#0", "B-0",
        "C-1", "C#1", "D-1", "D#1", "E-1", "F-1", "F#1", "G-1", "G#1", "A-1", "A#1", "B-1",
        "C-2", "C#2", "D-2", "D#2", "E-2", "F-2", "F#2", "G-2", "G#2", "A-2", "A#2", "B-2",
        "C-3", "C#3", "D-3", "D#3", "E-3", "F-3", "F#3", "G-3", "G#3", "A-3", "A#3", "B-3",
        "C-4", "C#4", "D-4", "D#4", "E-4", "F-4", "F#4", "G-4", "G#4", "A-4", "A#4", "B-4",
        "C-5", "C#5", "D-5", "D#5", "E-5", "F-5", "F#5", "G-5", "G#5", "A-5", "A#5", "B-5",
        "C-6", "C#6", "D-6", "D#6", "E-6", "F-6", "F#6", "G-6", "G#6", "A-6", "A#6", "B-6",
        "C-7", "C#7", "D-7", "D#7", "E-7", "F-7", "F#7", "G-7", "G#7", "A-7", "A#7", "B-7",
        "C-8", "C#8", "D-8", "D#8", "E-8", "F-8", "F#8", "G-8", "G#8", "A-8", "A#8", "B-8",
        "C-9", "C#9", "D-9", "D#9", "E-9", "F-9", "F#9", "G-9", "G#9", "A-9", "A#9", "B-9"
};
QString FastTracker2PatternView::note(BaseRow *row)
{
    if(row->note==255) return "()*";
    if(row->note==0) return "$%&";

    int note = row->note;
    note-=octaveOffset;
    if(note>=109 || note<0)
    {
        note = 0;
    }
    if(note==0)
    {
        return m_emptyNote;
    }
    else
    {
        return NOTES[note];
    }
}
QString FastTracker2PatternView::effect(BaseRow *row)
{
    if(row->effect<=15)
    {
        return AbstractPatternView::effect(row);
    }
    else if(row->effect==36)
    {
        return "-";
    }
    else
    {
        return QString(QChar(row->effect+55));
    }
}
QString FastTracker2PatternView::volume(BaseRow* row)
{
    //special
    if(row->effect2==2)
    {
        return "P" + QString::number(((row->param2-24) / 16) +1,16).toUpper();
    }
    else
    {
        if(!volumeEnabled) return "";
        int volume = row->vol;
        if(volume==0) return m_emptyVolume;
        int base = volumeHex ? 16 : 10;

        QString volumeStr = QString::number(volume,base).toUpper();
        volumeStr = volumePad && volumeStr.length()==1 ? "0" + volumeStr : volumeStr;
        return volumeStr;
    }
}
FastTracker2PatternView::~FastTracker2PatternView()
{

}
void FastTracker2PatternView::paintAbove(QPainter* painter, int height, int currentRow)
{

    QColor colorBase(73,117,130);
    QColor colorHilite(138,219,243);
    QColor colorShadow(24,40,44);

    painter->setPen(QColor(255,255,255));


    for(unsigned int chan = 0;chan<m_channels;chan++)
    {
        int extra=2;
        if(m_channels>4)
        {
            extra=0;
        }
        //bottom channels
        painter->fillRect((29)+chan*72*m_fontWidth/8,(height)-3,extra+70*m_fontWidth/8,1,colorHilite);
        //top channels
        painter->fillRect((29)+chan*72*m_fontWidth/8,31,extra+70*m_fontWidth/8,1,colorShadow);
        //channel number
        Tracker* t = (Tracker*)this->parent();

        t->drawText(QString::number(chan),painter,(28)+chan*72*m_fontWidth/8,43);
        //black line
        painter->fillRect((28)+chan*72*m_fontWidth/8,33,1,10,QColor(0,0,0));
    }
    //top
    painter->fillRect((1),30,53+(m_channels*72*m_fontWidth/8),1,colorBase);
    painter->fillRect((1),29,52+(m_channels*72*m_fontWidth/8),1,colorHilite);
    //bottom
    painter->fillRect((1),(height)-2,26+(m_channels*72*m_fontWidth/8),1,colorBase);
    painter->fillRect(0,(height)-1,55+(m_channels*72*m_fontWidth/8),1,colorShadow);

    //bottom left and right
    painter->fillRect((2),(height)-3,25,1,colorHilite);
    painter->fillRect(29+(m_channels*72*m_fontWidth/8),(height)-3,24,1,colorHilite);
    painter->fillRect(29+(m_channels*72*m_fontWidth/8),(height)-2,24,1,colorBase);

    //top left
    painter->fillRect((2),31,25,1,colorShadow);
    //top right
    painter->fillRect(29+(m_channels*72*m_fontWidth/8),31,25,1,colorShadow);
}

void FastTracker2PatternView::paintBelow(QPainter* painter, int height, int currentRow)
{
    int left=3;

    QColor colorBase(73,117,130);
    QColor colorHilite(138,219,243);
    QColor colorShadow(24,40,44);
    QPen pen(colorBase);
    pen.setWidth(1);
    painter->setPen(pen);
    int topOffset=-4;


    //left border
    pen.setColor(colorHilite);
    painter->setPen(pen);
    painter->drawLine(((left-2)),29,((left-2)),height);
    pen.setColor(colorBase);
    painter->setPen(pen);
    painter->drawLine(((left-1)),29,((left-1)),height);
    pen.setColor(colorShadow);
    painter->setPen(pen);
    painter->drawLine(((left)),29,((left)),height-4);

    for(unsigned int chan = 0;chan<=m_channels;chan++)
    {
        //channel dividers
        pen.setColor(colorHilite);
        painter->setPen(pen);
        painter->drawLine((left+24+chan*72*m_fontWidth/8),29,(left+24+chan*72*m_fontWidth/8),height);
        pen.setColor(colorBase);
        painter->setPen(pen);
        painter->drawLine((left+25+chan*72*m_fontWidth/8),29,(left+25+chan*72*m_fontWidth/8),height);
        pen.setColor(colorShadow);
        painter->setPen(pen);
        painter->drawLine((left+26+chan*72*m_fontWidth/8),29,(left+26+chan*72*m_fontWidth/8),(height)-2);
    }

    //right border
    pen.setColor(colorHilite);
    painter->setPen(pen);
    painter->drawLine((54)+(m_channels*72*m_fontWidth/8),29,(54)+(m_channels*72*m_fontWidth/8),height);
    pen.setColor(colorBase);
    painter->setPen(pen);
    painter->drawLine((55)+(m_channels*72*m_fontWidth/8),29,(55)+(m_channels*72*m_fontWidth/8),height);
    pen.setColor(colorShadow);
    painter->setPen(pen);
    painter->drawLine((56)+(m_channels*72*m_fontWidth/8),29,(56)+(m_channels*72*m_fontWidth/8),height);

    for(unsigned int chan = 0;chan<m_channels;chan++)
    {
        //top hilite
        int extra=2;
        if(m_channels>4)
        {
            extra=0;
        }
        painter->fillRect(((left+26))+chan*72*m_fontWidth/8,(height/2)-6+(topOffset),extra+70*m_fontWidth/8,1,colorHilite);
        //bottom shadow
        painter->fillRect(((left+26))+chan*72*m_fontWidth/8,(height/2)+4+(topOffset),extra+70*m_fontWidth/8,1,colorShadow);

    }


    //current row left
    //top hilite
    painter->fillRect(((left-1)),(height/2)-6+(topOffset),24,1,colorHilite);
    //bottom shadow
    painter->fillRect(((left-1)),(height/2)+4+(topOffset),25,1,colorShadow);

    //current row right
    //top hilite
    painter->fillRect(((left+25))+m_channels*72*m_fontWidth/8,(height/2)-6+(topOffset),25,1,colorHilite);
    //bottom shadow
    painter->fillRect(((left+26))+m_channels*72*m_fontWidth/8,(height/2)+4+(topOffset),25,1,colorShadow);

    //main current row
    painter->fillRect(((left-1)),(height/2)-9,(52)+(m_channels*72*m_fontWidth/8),9,colorBase);




}
