#include "SoundFXPatternView.h"

SoundFXPatternView::SoundFXPatternView(Tracker *parent, unsigned int channels, int scale)
                            :AbstractPatternView(parent,channels,scale)
{
    m_font = QFont("Sound FX V1.8");
    m_font.setPixelSize(16);
    m_fontWidth = 8;
    m_font.setStyleStrategy(QFont::NoAntialias);

    m_bitmapFont = BitmapFont("Sound FX V1.8");
    m_fontWidth = 8;
    m_fontHeight = 16;


    m_ColorWindowBackground = "00ffee";
    m_ColorRowNumberBackground = m_ColorWindowBackground;
    m_colorDefault = m_ColorRowNumber = m_ColorInstrument = m_ColorEffect = m_ColorParameter = m_ColorEffect2 = m_ColorParameter2 = m_ColorVolume = m_colorEmpty = QColor(0,0,204);
    m_colorCurrentRowBackground = QColor(0,255,0);
    m_colorCurrentRowForeground = QColor(255,0,0);
    m_SeparatorChannel = "'''";
    m_RowNumbersEveryChannelEnabled = true;
    instrumentPad = false;
    m_emptyInstrument = "0";
    m_SeparatorNote = "''";
    m_SeparatorParameter = "''";
    m_SeparatorRowNumber = ".''";
    m_xOffsetRow=16;
    m_RowLength = 80;
    m_xChannelStart = 3;
    m_channelWidth = 152;
    m_channelxSpace = 0;
    m_colorBackground = QColor(0,255,238,220);
    octaveOffset = 44;
}

SoundFXPatternView::~SoundFXPatternView()
{

}
void SoundFXPatternView::paintAbove(QPainter* painter, int height, int currentRow)
{
    QColor colorCyan = QColor(0,255,238);
    //top
    painter->fillRect(1,2,609,2,QColor(0,0,0));
    //bottom line
    painter->fillRect(1,height-(3),609,2,QColor(0,0,0));

    //bottom blue
    painter->fillRect(16,height-(1),570,2,colorCyan);
    //top blue
    painter->fillRect(16,0,570,2,colorCyan);
}

void SoundFXPatternView::paintBelow(QPainter* painter, int height, int currentRow)
{
    QColor colorCyan = QColor(0,255,238);
    //background
    painter->fillRect(0,0,640,height,colorCyan);

    //channel separators
    for(unsigned int chan = 0;chan<m_channels+1;chan++)
    {
        int xtra = chan==m_channels ? 1 : 0;
        painter->fillRect((1+chan*152)-(xtra),4,2,height-(5),QColor(0,0,0));
    }


    painter->fillRect(1,(height/2)-17,607,18,QColor(0,0,0));
    painter->fillRect(2,(height/2)-15,606,14,m_colorCurrentRowBackground);

    //scrollbar
    painter->fillRect(611,4,1,height-(5),QColor(0,0,0));
    painter->fillRect(639,4,1,height-(5),QColor(0,0,0));
    painter->fillRect(611,2,29,2,QColor(0,0,0));
    painter->fillRect(611,height-(3),29,2,QColor(0,0,0));

    float currentRowPos = currentRow/64.0;
    int yPos = (currentRowPos*(height-16))+4;
    painter->fillRect(614,yPos,23,8,m_colorDefault);
}

QString SoundFXPatternView::note(BaseRow* row)
{
    QString note = AbstractPatternView::note(row);
    if(note!="---")
    {
        note.replace('-',' ');
    }
    return note;
}
