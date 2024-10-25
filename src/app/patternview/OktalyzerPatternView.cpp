#include "OktalyzerPatternView.h"

OktalyzerPatternView::OktalyzerPatternView(Tracker* parent, unsigned int channels, int scale)
    : AbstractPatternView(parent, channels, scale)
{
    m_font = QFont("Oktalyzer");
    m_font.setPixelSize(16);
    m_fontWidth = 8;

    m_bitmapFont = BitmapFont("Oktalyzer");
    m_fontWidth = 8;
    m_fontHeight = 16;

    m_font.setStyleStrategy(QFont::NoAntialias);
    octaveOffset = 48;
    rowNumberHex = true;
    rowNumberOffset = 0;
    instrumentOffset = -1;
    m_ColorWindowBackground = "000044";
    m_ColorRowNumberBackground = m_ColorWindowBackground;
    m_colorCurrentRowForeground = m_colorDefault = m_ColorRowNumber = m_ColorInstrument = m_ColorEffect =
        m_ColorParameter = m_ColorEffect2 = m_ColorParameter2 = m_ColorVolume = m_colorEmpty = QColor(17, 153, 238);
    m_colorCurrentRowBackground = QColor(17, 153, 238);

    m_RowEnd = "'";
    m_SeparatorChannel = "'";
    m_RowStart = "00'";

    instrumentPad = false;
    m_emptyInstrument = "0";
    m_SeparatorNote = "'";
    m_SeparatorParameter = "";
    m_SeparatorRowNumber = "'";

    m_xOffsetRow = 1;
    m_RowLength = 6 + m_channels * 9;

    m_xChannelStart = 40;
    m_channelWidth = 72;
    m_channelLastWidth = 73;
    m_channelxSpace = 0;
    m_colorBackground = QColor(0, 0, 66, 180);
    m_topHeight = 0;
    m_bottomFrameHeight = 0;
}

void OktalyzerPatternView::paintBelow(QPainter* painter, int height, int currentRow)
{
    //background
    painter->fillRect(0, 0, (41) + (m_channels * 72), height, QColor(0, 0, 66));
    //main
    painter->fillRect(0, (height / 2) - 17, (40) + (m_channels * 72), 2, m_colorCurrentRowBackground);
    painter->fillRect(0, (height / 2) - 1, (40) + (m_channels * 72), 2, m_colorCurrentRowBackground);
    painter->fillRect((40) + (m_channels * 72), (height / 2) - 17, 1, 18, m_colorCurrentRowBackground);
    painter->fillRect(0, (height / 2) - 17, 1, 18, m_colorCurrentRowBackground);
}

QString OktalyzerPatternView::effect(BaseRow* row)
{
    QString effectStr;
    if (row->effect < 10)
    {
        return QString::number(row->effect);
    }
    switch (row->effect)
    {
    case 112: effectStr = "A";
        break;
    case 113: effectStr = "B";
        break;
    case 114: effectStr = "C";
        break;
    case 156: effectStr = "D";
        break;
    case 157: effectStr = "H";
        break;
    case 117: effectStr = "L";
        break; //117 for Z also?
    case 11: effectStr = "P";
        break;
    case 15: effectStr = "S";
        break;
    case 118: effectStr = "U";
        break;
    case 12:
    case 174:
    case 10:
        effectStr = "V";
        break;
    case 69: effectStr = "W";
        break;
    case 110: effectStr = "X";
        break;
    case 32: effectStr = "Y";
        break;

    default: effectStr = QString::number(row->effect);
        break;
    }
    return effectStr;
}

QString OktalyzerPatternView::instrument(BaseRow* row)
{
    int instrument = row->sample;
    if (instrument == 0) return m_emptyInstrument;
    return QString::number(instrument - 1, 36).toUpper();
}

OktalyzerPatternView::~OktalyzerPatternView()
{
}
