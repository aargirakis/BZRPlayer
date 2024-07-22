#include "MEDPatternView.h"

MEDPatternView::MEDPatternView(Tracker *parent, unsigned int channels, int scale)
                            :AbstractPatternView(parent,channels,scale)
{
    octaveOffset = 12;
    m_font = QFont("MED 3.21");
    m_font.setPixelSize(16);
    m_fontWidth = 16;

    m_bitmapFont = BitmapFont("MED 3.21");
    m_bitmapFont3 = BitmapFont("MED 3.21 Compressed");
    m_fontWidth = 16;
    m_fontHeight = 16;

    rowNumberOffset = 0;
    m_emptyInstrument="0";
    instrumentPad = false;
    m_colorCurrentRowBackground = QColor(102,102,119);
    m_colorCurrentRowForeground = QColor(153,153,170);
    m_colorDefault = m_ColorRowNumber = m_ColorInstrument = m_ColorEffect = m_ColorParameter = m_ColorVolume = m_colorEmpty = QColor(204,204,204);

    m_SeparatorRowNumber = m_SeparatorChannel = "'";
    m_RowEnd = "'";
    m_RowLength = 40;
    m_xOffsetRow=8;

    m_xChannelStart = 56;
    m_channelWidth = 144;
    m_channelxSpace = 0;
    m_width=640;
    m_height=512;

}
BitmapFont MEDPatternView::infoFont()
{
    return m_bitmapFont3;
}
QString MEDPatternView::rowNumber(int rowNumber)
{
    int base = rowNumberHex ? 16 : 10;

    QString rowNumberStr = QString::number(rowNumber+rowNumberOffset,base).toUpper();
    rowNumberStr = rowNumberStr.length()==1 ? "00" + rowNumberStr : rowNumberStr;
    rowNumberStr = rowNumberStr.length()==2 ? "0" + rowNumberStr : rowNumberStr;
    return rowNumberStr;
}
QString MEDPatternView::effect(BaseRow* row)
{
    if(row->effect==171) return "F"; //might be a bug in libxmp
    if(row->effect==146) return "4"; //might be a bug in libxmp
    return AbstractPatternView::effect(row);
}
QString MEDPatternView::note(BaseRow* row)
{
    //hold/decay
    if(row->effect2==177)
    {
        return "-|-";
    }
    QString note = AbstractPatternView::note(row);
    if(note.left(1)=="B")
    {
        note = "H" + note.right(2);
    }
    return note;
}
QString MEDPatternView::instrument(BaseRow* row)
{
    int instrument = row->sample;
    QString padding = m_SeparatorRowNumber;
    if(instrument>=32)
    {
        padding="";
    }
    return padding+QString::number(instrument,32).toUpper();
}
void MEDPatternView::paintAbove(QPainter* painter, int height, int currentRow)
{

    QColor colorBase(153,153,170);
    QColor colorHilite(204,204,204);
    QColor colorShadow(102,102,119);

    //bottom
    painter->fillRect(8,(height)-8,625,2,colorHilite);
    painter->fillRect(6,(height)-6,628,2,colorBase);
    painter->fillRect(4,(height)-4,633,2,colorShadow);
    painter->fillRect(4,(height)-2,634,2,colorShadow);

    //vumeter bottom
    for(unsigned int i = 0;i<m_channels;i++)
    {
        painter->fillRect((152)+144*i,(height)-(8),4,2,QColor(170,170,170));
        painter->fillRect((156)+144*i,(height)-(8),16,2,QColor(0,136,0));
        painter->fillRect((172)+144*i,(height)-(8),4,2,QColor(187,187,187));
    }
}

void MEDPatternView::paintBelow(QPainter* painter, int height, int currentRow)
{

    QColor colorBase(153,153,170);
    QColor colorHilite(204,204,204);
    QColor colorShadow(102,102,119);
    QPen pen(colorBase);
    pen.setWidth(1);
    painter->setPen(pen);

    painter->fillRect(8,(height/2)-17,628,18,m_colorCurrentRowBackground);

    //left border

    painter->fillRect(0,0,4,height,colorShadow);
    painter->fillRect(4,0,2,height,colorBase);
    painter->fillRect(6,0,2,height,colorHilite);


    //right border
    painter->fillRect(636,0,4,height,colorHilite);
    painter->fillRect(634,0,2,height,colorBase);
    painter->fillRect(632,0,2,height,colorShadow);

}
MEDPatternView::~MEDPatternView()
{

}
