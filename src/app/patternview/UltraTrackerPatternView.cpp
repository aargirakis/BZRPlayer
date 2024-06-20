#include "UltraTrackerPatternView.h"

UltraTrackerPatternView::UltraTrackerPatternView(Tracker *parent, unsigned int channels, int scale)
                            :AbstractPatternView(parent,channels,scale)
{
    octaveOffset = 48;
    rowNumberOffset = 0;
    m_font = QFont("Ultra Tracker");
    m_font.setPixelSize(8);
    m_fontWidth = 8;

    m_bitmapFont = BitmapFont("Ultra Tracker");
    m_fontWidth = 8;
    m_fontHeight = 8;

    effect2Enabled = true;
    parameter2Enabled = true;
    m_colorCurrentRowBackground = QColor(0,0,170);
    m_colorCurrentRowForeground = QColor(255,255,255);
    m_colorCurrentRowHighlightForeground = QColor(243,243,44);
    m_colorRowHighlightForeground = QColor(40,251,89);
    m_RowHighlightForegroundFrequency=16;
    m_CurrentRowHighlightForegroundFrequency = 16;
    m_effectsThenParametersEnabled = true;


    m_colorDefault = m_ColorRowNumber = m_ColorInstrument = m_ColorEffect = m_ColorParameter = m_ColorEffect2 = m_ColorParameter2 = m_ColorVolume = m_colorEmpty = QColor(0,170,0);

    m_instrumentPaddingCharacter = "-";
    m_SeparatorInstrument="'";
    m_SeparatorVolume ="";
    m_SeparatorNote ="'";
    m_SeparatorEffect2 ="'";
    m_RowEnd = m_SeparatorRowNumber = m_SeparatorChannel = "'";

    m_emptyNote = "---";
    m_emptyVolume = "--";
    m_emptyEffect = "-";
    m_emptyInstrument = "--";
    m_emptyParameter = "--";

    m_RowLength = (5 + m_channels*15);
    m_xOffsetRow=16;

    m_xChannelStart = 37;
    m_channelWidth = 118;
    m_channelxSpace = 2;
    m_bottomFrameHeight=3;
    m_topHeight=4;
}

UltraTrackerPatternView::~UltraTrackerPatternView()
{

}
void UltraTrackerPatternView::paintAbove(QPainter* painter, int height, int currentRow)
{
    QColor colorGrey(170,170,170);

    //left
    painter->fillRect(3,0,2,height-(3),colorGrey);

    //channel separators
    for(unsigned int chan = 0;chan<m_channels+1;chan++)
    {
        painter->fillRect((35+chan*120),0,2,height-(3),colorGrey);
    }
    //top
    painter->fillRect(3,3,(34+m_channels*120),1,colorGrey);
    painter->fillRect(3,0,(34+m_channels*120),3,QColor(0,0,0));
    //bottom
    painter->fillRect(3,(height)-3,(34+m_channels*120),1,colorGrey);
    painter->fillRect(3,(height)-2,(34+m_channels*120),3,QColor(0,0,0));
}
void UltraTrackerPatternView::paintBelow(QPainter* painter, int height, int currentRow)
{
    painter->fillRect(0,(height/2)-8,(42+m_channels*120),8,m_colorCurrentRowBackground);
}
QString UltraTrackerPatternView::note(BaseRow* row)
{
    int note = row->note;
    switch (note)
    {
        case 37: return "C--";
        case 38: return "C#-";
        case 39: return "D--";
        case 40: return "D#-";
        case 41: return "E--";
        case 42: return "F--";
        case 43: return "F#-";
        case 44: return "G--";
        case 45: return "G#-";
        case 46: return "A--";
        case 47: return "A#-";
    }
    return AbstractPatternView::note(row);
}

QString UltraTrackerPatternView::parameter(BaseRow* row)
{
    return AbstractPatternView::parameter(row).replace("0","-");
}

QString UltraTrackerPatternView::parameter2(BaseRow* row)
{
    return AbstractPatternView::parameter2(row).replace("0","-");
}
