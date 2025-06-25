#include "ScreamTracker2PatternView.h"

ScreamTracker2PatternView::ScreamTracker2PatternView(Tracker* parent, unsigned int channels)
    : AbstractPatternView(parent, channels)
{
    octaveOffset = 48;
    rowNumberOffset = 0;
    instrumentHex = false;
    volumeEnabled = true;
    m_emptyInstrument = "\"";
    m_font = QFont("Scream Tracker 2");
    m_font.setPixelSize(16);

    m_bitmapFont = BitmapFont("Scream Tracker 2");
    m_fontWidth = 8;
    m_fontHeight = 16;

    m_fontWidth = 8;
    m_colorCurrentRowBackground = QColor(0, 170, 170);
    m_colorCurrentRowForeground = QColor(0, 0, 0);

    m_colorDefault = m_ColorRowNumber = m_ColorInstrument = m_ColorEffect = m_ColorParameter = m_ColorEffect2 =
        m_ColorParameter2 = m_ColorVolume = m_colorEmpty = QColor(168, 168, 168);

    m_SeparatorInstrument = "'";
    m_SeparatorVolume = "'";
    m_SeparatorNote = "'";
    m_SeparatorChannel = "'";
    m_SeparatorRowNumber = m_RowEnd = m_SeparatorChannel;

    m_emptyNote = "\"\"\"";
    m_emptyVolume = "\"\"";
    m_emptyEffect = ".";
    m_emptyInstrument = "\"\"";

    m_ColorRowHighlightBackground = QColor(34, 34, 34);
    m_RowHighlightBackgroundFrequency = 4;

    m_xOffsetRow = 8;
    m_RowLength = 60 + 1;
    m_highlightBackgroundOffset = 30;
    m_ColorRowNumberCurrentRow = QColor(168, 168, 168);
    m_ColorRowNumberCurrentRowEnabled = true;
    m_yOffsetRowHighlight = -15;

    m_xChannelStart = 32;
    m_channelWidth = 104;
    m_channelxSpace = 8;
    m_width = 640;
    m_height = 480;
    m_bottomFrameHeight = 16;
    m_topHeight = 16;
}

void ScreamTracker2PatternView::paintAbove(QPainter* painter, int height, int currentRow)
{
    QColor colorBrown(168, 84, 0);
    QColor colorBlue(0, 0, 168);

    //left
    painter->fillRect(0, 0, 8, height, colorBrown);
    for (int chan = 0; chan < m_channels; chan++)
    {
        painter->fillRect((24 + chan * 112), 0, 8, height, colorBrown);
    }
    //right
    painter->fillRect(472, 0, 16, height, colorBrown);
    //bottom
    painter->fillRect(0, height - 16, 488, 16, colorBrown);
    //top
    painter->fillRect(0, 0, 488, 16, colorBlue);
}

void ScreamTracker2PatternView::paintBelow(QPainter* painter, int height, int currentRow)
{
    //main
    painter->fillRect(30, (height / 2) - 15, 442, 16, m_colorCurrentRowBackground);
}

ScreamTracker2PatternView::~ScreamTracker2PatternView()
{
}

QString ScreamTracker2PatternView::note(BaseRow* row)
{
    int note = row->note;
    note -= octaveOffset;
    if (note >= 109 || note < 0)
    {
        note = 0;
    }
    if (note == 0)
    {
        if (row->sample > 0)
        {
            return "C-0";
        }
        else
        {
            return m_emptyNote;
        }
    }
    else
    {
        return NOTES[note];
    }
}

QString ScreamTracker2PatternView::parameter(BaseRow* row)
{
    int parameter = row->param;
    if (parameter == 0) return m_emptyParameter;
    int base = parameterHex ? 16 : 10;

    QString parameterStr = QString::number(parameter, base).toUpper();
    parameterStr = parameterPad && parameterStr.length() == 1 ? "0" + parameterStr : parameterStr;
    if (row->effect == 15) //parameter for A-effect is shown as X0 instead of 0X?
    {
        QString first = parameterStr.at(0);
        parameterStr[0] = parameterStr.at(1);
        parameterStr[1] = first.at(0);
    }
    return parameterStr;
}

QString ScreamTracker2PatternView::effect(BaseRow* row)
{
    QString effectStr;
    if (row->effect < 0) return "ERROR";
    switch (row->effect)
    {
    case 15: effectStr = "A";
        break;
    case 11: effectStr = "B";
        break;
    case 13: effectStr = "C";
        break;
    case 10: effectStr = "D";
        break;
    case 2: effectStr = "E";
        break;
    case 1: effectStr = "F";
        break;
    case 3: effectStr = "G";
        break;
    case 4: effectStr = "H";
        break;
    case 0: if (row->param > 0) { effectStr = "J"; }
        else { effectStr = m_emptyEffect; };
        break;
    default: effectStr = "I";
        break; //The only missing official effect is I, I don't know the number for it though
    }
    return effectStr;
}
