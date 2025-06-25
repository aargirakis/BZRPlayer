#include "ImpulseTrackerPatternView.h"

ImpulseTrackerPatternView::ImpulseTrackerPatternView(Tracker* parent, unsigned int channels)
    : AbstractPatternView(parent, channels)
{
    octaveOffset = 12;
    volumeEnabled = true;
    instrumentHex = false;
    rowNumberOffset = 0;
    m_font.setPixelSize(8);
    m_fontWidth = 8;
    m_colorCurrentRowBackground = QColor(235, 235, 203);
    m_colorCurrentRowForeground = QColor(0, 0, 0);

    m_colorDefault = m_ColorInstrument = m_ColorEffect = m_ColorParameter = m_ColorVolume = m_colorEmpty =
        QColor(69, 154, 73);
    m_ColorRowNumber = QColor(0, 0, 0);
    //m_ColorRowNumberBackground ="b69679";

    m_SeparatorInstrument = "'";
    m_SeparatorVolume = "'";
    m_SeparatorNote = "'";
    m_SeparatorChannel = "'";
    m_SeparatorRowNumber = "'";
    m_RowEnd = "'";

    m_emptyNote = "\"\"\"";
    m_emptyVolume = "\"\"";
    m_emptyEffect = ".";
    m_emptyInstrument = "\"\"";

    m_ColorRowHighlightBackground = QColor(21, 19, 18);
    m_RowHighlightBackgroundFrequency = 4;

    m_ColorRowHighlightBackground2 = QColor(36, 26, 34);
    m_RowHighlightBackgroundFrequency2 = 16;

    m_highlightBackgroundOffset = 30;
    m_yOffsetRowHighlight = -7;

    m_xOffsetRow = 8;
    m_RowLength = (5 + m_channels * 14) + 5;

    m_xChannelStart = 39;
    m_channelWidth = 106;
    m_channelxSpace = 6;
    m_width = 640;
    m_height = 480;
    m_bottomFrameHeight = 24;
    m_topHeight = 16;
}

void ImpulseTrackerPatternView::paintAbove(QPainter* painter, int height, int currentRow)
{
    QColor colorBase(180, 148, 120);
    QColor colorHilite(232, 232, 200);
    QColor colorShadow(124, 88, 68);
    QColor colorRed(255, 38, 38);

    //channel separators
    for (unsigned int chan = 1; chan < m_channels; chan++)
    {
        painter->fillRect((33 + chan * 112), 0, 6, height, colorBase);
    }
    //right
    painter->fillRect((34 + m_channels * 112), 0, 46, height, colorBase);
    painter->fillRect((32 + m_channels * 112), 16, 2, height - (14), colorHilite);

    //bottom
    painter->fillRect(40, height - 24, (m_channels * 112 - 6), 2, colorHilite);
    painter->fillRect(8, height - 22, (32 + m_channels * 112), 22, colorBase);

    //left
    painter->fillRect(0, 0, 1, height, colorHilite);
    painter->fillRect(38, 0, 2, height - (21), colorShadow);

    //top
    painter->fillRect(39, 14, (m_channels * 112 - 5), 2, colorShadow);
    painter->fillRect(8, 1, (32 + m_channels * 112), 13, colorBase);
    painter->fillRect(0, 0, (80 + m_channels * 112), 1, colorHilite);
}

void ImpulseTrackerPatternView::paintBelow(QPainter* painter, int height, int currentRow)
{
    QColor colorBase(180, 148, 120);
    QColor colorHilite(232, 232, 200);
    QColor colorShadow(124, 88, 68);
    //main
    painter->fillRect(40, (height / 2) - 7, (40 + m_channels * 112), 8, m_colorCurrentRowBackground);

    //left
    painter->fillRect(1, 0, 39, height, colorBase);
}

ImpulseTrackerPatternView::~ImpulseTrackerPatternView()
{
}

QString ImpulseTrackerPatternView::volume(BaseRow* row)
{
    if (!volumeEnabled) return "";
    int volume = row->vol;
    if (volume < 1) volume = -1;
    if (volume == -1) return m_emptyVolume;
    int base = volumeHex ? 16 : 10;

    QString volumeStr = QString::number(volume, base).toUpper();
    volumeStr = volumePad && volumeStr.length() == 1 ? "0" + volumeStr : volumeStr;
    return volumeStr;
}

QString ImpulseTrackerPatternView::rowNumber(int rowNumber)
{
    int base = rowNumberHex ? 16 : 10;

    QString rowNumberStr = QString::number(rowNumber + rowNumberOffset, base).toUpper();
    rowNumberStr = rowNumberStr.length() == 1 ? "00" + rowNumberStr : rowNumberStr;
    rowNumberStr = rowNumberStr.length() == 2 ? "0" + rowNumberStr : rowNumberStr;
    return rowNumberStr;
}

QString ImpulseTrackerPatternView::note(BaseRow* row)
{
    if (row->note == 130) return "$$$";
    return AbstractPatternView::note(row);
}

QString ImpulseTrackerPatternView::effect(BaseRow* row)
{
    QString effectStr;
    //std::cout << QString::number(row->effect+1).toStdString() << std::endl;
    if (row->effect < 1) return m_emptyEffect;
    switch (row->effect + 1)
    {
    case 2: effectStr = "F";
        break;
    case 3: effectStr = "E";
        break;
    case 4: effectStr = "G";
        break;
    case 5: effectStr = "H";
        break;
    case 6: effectStr = "L";
        break;
    case 7: effectStr = "K";
        break;
    case 8: effectStr = "R";
        break;
    case 9: effectStr = "X";
        break;
    case 10: effectStr = "O";
        break;
    case 11: effectStr = "D";
        break;
    case 12: effectStr = "B";
        break;
    case 14: effectStr = "C";
        break;
    case 15: effectStr = "S";
        break;
    case 16: effectStr = "A";
        break;
    case 17: effectStr = "V";
        break;

    case 20: effectStr = "S";
        break;
    case 21: effectStr = "M";
        break;
    case 22: effectStr = "N";
        break;
    case 23: effectStr = "X";
        break;
    case 24: effectStr = "W";
        break;
    case 26: effectStr = "U";
        break;
    case 27: effectStr = "X";
        break;
    case 28: effectStr = "Q";
        break;
    case 29: effectStr = "P";
        break;
    case 30: effectStr = "I";
        break;
    case 31: effectStr = "Y";
        break;
    case 37: effectStr = m_emptyEffect;
        break;

    case 133: effectStr = "Z";
        break;
    case 136: effectStr = "T";
        break;


    case 172: effectStr = "T";
        break;

    case 181: effectStr = "J";
        break;
    default: effectStr = "???";
        break;
    }
    return effectStr;
}
