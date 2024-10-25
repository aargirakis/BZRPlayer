#include "MultiTrackerPatternView.h"

MultiTrackerPatternView::MultiTrackerPatternView(Tracker* parent, unsigned int channels, int scale)
    : AbstractPatternView(parent, channels, scale)
{
    m_font = QFont("MultiTracker");
    m_font.setPixelSize(8);
    m_fontWidth = 8;

    m_bitmapFont = BitmapFont("MultiTracker");
    m_fontWidth = 8;
    m_fontHeight = 8;


    octaveOffset = 48;
    instrumentPad = false;
    rowNumberOffset = 0;
    volumeHex = true;
    rowNumberPad = true;
    m_colorDefault = m_ColorRowNumber = m_ColorInstrument = m_ColorEffect = m_ColorParameter = m_ColorEffect2 =
        m_ColorParameter2 = m_ColorVolume = m_colorEmpty = QColor(0, 85, 170);
    m_ColorRowNumber = QColor(255, 255, 255);
    m_ColorRowNumberCurrentRowEnabled = true;
    m_ColorRowNumberCurrentRow = QColor(255, 255, 255);

    m_colorDefaultAlternate = QColor(0, 85, 170);
    m_colorEmptyAlternate = QColor(0, 85, 170);
    m_ColorInstrumentAlternate = QColor(0, 170, 255);
    m_ColorEffectAlternate = QColor(0, 0, 170);
    m_ColorParameterAlternate = QColor(0, 0, 170);
    m_colorDefault = QColor(85, 85, 255);
    m_colorEmpty = QColor(85, 85, 255);
    m_ColorInstrument = QColor(85, 170, 255);
    m_ColorEffect = QColor(85, 0, 255);
    m_ColorParameter = QColor(85, 0, 255);
    m_emptyInstrument = "0";
    m_emptyParameter = "00";
    m_emptyNote = "---";
    m_emptyEffect = "0";
    m_colorCurrentRowBackground = QColor(0, 0, 85);
    m_colorCurrentRowForeground = QColor(0, 170, 255);
    m_alternateChannelColorsFrequency = 2;

    m_SeparatorChannel = m_SeparatorInstrument = m_SeparatorRowNumber = "'";
    m_noEmptyInstrumentColor = true;
    m_noEmptyEffectColor = true;
    m_noEmptyParameterColor = true;
    m_useInstrumentColorOnEffectAndParameterFrequency = 4;
    m_xOffsetRow = 16;

    m_RowLength = 4 + (m_channels * 9) + 4;
    m_xChannelStart = 40;
    m_channelFirstWidth = 67;
    m_channelWidth = 70;
    m_channelxSpace = 2;
    m_bottomFrameHeight = 16;
    m_topHeight = 25;
}

MultiTrackerPatternView::~MultiTrackerPatternView()
{
}

void MultiTrackerPatternView::paintAbove(QPainter* painter, int height, int currentRow)
{
    //bottom
    painter->fillRect(0, height - 16, (40 + m_channels * 72), 16, QColor(0, 0, 85));
}

void MultiTrackerPatternView::paintBelow(QPainter* painter, int height, int currentRow)
{
    //left
    painter->fillRect(0, 0, 40, height, QColor(0, 0, 85));
    //right
    painter->fillRect((40 + m_channels * 72), 0, 24, height, QColor(0, 0, 85));

    //main
    painter->fillRect(16, (height / 2) - 8, 16, 8, QColor(0, 170, 255));
    painter->fillRect(40, (height / 2) - 8, ((m_channels * 72) - 4), 8, QColor(0, 0, 85));
    //channel separators
    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        painter->fillRect((75 + chan * 72), 0, 2, height, QColor(255, 255, 255));
        painter->fillRect((107 + chan * 72), 0, 2, height, QColor(255, 255, 255));
        //current row channels separators
        painter->fillRect((75 + chan * 72), (height / 2) - 8, 2, 8, QColor(0, 170, 255));
        painter->fillRect((107 + chan * 72), (height / 2) - 8, 2, 8, QColor(0, 170, 255));
    }
}

QString MultiTrackerPatternView::rowNumber(int rowNumber)
{
    int base = rowNumberHex ? 16 : 10;

    QString rowNumberStr = QString::number(rowNumber + rowNumberOffset, base).toUpper();
    rowNumberStr = rowNumberPad && rowNumberStr.length() == 1 ? rowNumberStr + "'" : rowNumberStr;
    return rowNumberStr;
}

QString MultiTrackerPatternView::instrument(BaseRow* row)
{
    int instrument = row->sample;
    return QString::number(instrument, 32).toUpper();
}

QString MultiTrackerPatternView::effect(BaseRow* row)
{
    if (row->effect == 0x8)
    {
        return "e";
    }
    return AbstractPatternView::effect(row).toLower();
}

QString MultiTrackerPatternView::parameter(BaseRow* row)
{
    int parameter = row->param;
    if (row->effect == 0x8)
    {
        int base = parameterHex ? 16 : 10;

        QString parameterStr = QString::number(parameter, base).toLower();
        parameterStr = "8" + parameterStr.left(1);
        return parameterStr;
    }
    return AbstractPatternView::parameter(row).toLower();
}

QString MultiTrackerPatternView::note(BaseRow* row)
{
    int note = row->note;
    note -= octaveOffset;
    if (note >= 109 || note < 0)
    {
        note = 0;
    }
    if (note == 0)
    {
        return m_emptyNote;
    }
    else
    {
        return QString(NOTES[note]).replace("-", "'");
    }
}
