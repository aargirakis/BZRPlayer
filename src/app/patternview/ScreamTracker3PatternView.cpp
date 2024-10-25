#include "ScreamTracker3PatternView.h"
#include <QApplication>
#include <QDir>

ScreamTracker3PatternView::ScreamTracker3PatternView(Tracker* parent, unsigned int channels, int scale)
    : AbstractPatternView(parent, channels, scale)
{
    octaveOffset = 24;
    volumeEnabled = true;
    rowNumberOffset = 0;
    m_font.setPixelSize(8);
    m_fontWidth = 8;
    m_colorCurrentRowBackground = QColor(255, 223, 134);
    m_colorCurrentRowForeground = QColor(0, 0, 0);

    m_colorDefault = m_ColorInstrument = m_ColorEffect = m_ColorParameter = m_ColorVolume = m_colorEmpty =
        QColor(152, 148, 144);

    m_ColorRowNumber = QColor(80, 68, 40);
    //m_ColorRowNumberBackground ="a49054";

    m_SeparatorInstrument = "'";
    m_SeparatorVolume = "'";
    m_SeparatorNote = "'";
    m_SeparatorChannel = m_SeparatorRowNumber = m_RowEnd = "'";
    m_emptyNote = "\"\"\"";
    m_emptyVolume = "\"\"";
    m_emptyEffect = ".";
    m_emptyInstrument = "\"\"";

    m_ColorRowHighlightBackground = QColor(48, 44, 40);
    m_RowHighlightBackgroundFrequency = 4;

    m_ColorRowHighlightBackground2 = QColor(73, 65, 60);
    m_RowHighlightBackgroundFrequency2 = 16;
    m_ColorRowNumberCurrentRow = QColor(80, 68, 40);
    m_ColorRowNumberCurrentRowEnabled = true;
    m_highlightBackgroundOffset = 30;
    m_yOffsetRowHighlight = -7;

    m_xOffsetRow = 16;
    m_RowLength = (5 + m_channels * 14) + 5;

    m_xChannelStart = 40;
    m_channelWidth = 108;
    m_channelLastWidth = 104;
    m_channelxSpace = 4;
    m_width = 640;
    m_height = 480;
    m_bottomFrameHeight = 16;
    m_topHeight = 16;
}

void ScreamTracker3PatternView::paintAbove(QPainter* painter, int height, int currentRow)
{
    QColor colorBase(164, 144, 84);
    QColor colorHilite(252, 220, 132);
    QColor colorShadow(80, 68, 40);

    //channel separators
    for (unsigned int chan = 1; chan < m_channels; chan++)
    {
        painter->fillRect((32 + chan * 112), 0, 2, height, QColor(0, 0, 0));
        painter->fillRect((34 + chan * 112), 0, 4, height, colorBase);
        painter->fillRect((38 + chan * 112), 0, 2, height, QColor(0, 0, 0));
    }
    //right

    painter->fillRect((32 + m_channels * 112), 0, 46, height, colorBase);
    painter->fillRect((32 + m_channels * 112), 16, 2, height - (14), colorHilite);
    painter->fillRect((78 + m_channels * 112), 0, 2, height - (1), colorShadow);

    //bottom
    painter->fillRect(40, height - 16, (m_channels * 112 - 6), 2, colorHilite);
    painter->fillRect(8, height - 14, (32 + m_channels * 112), 12, colorBase);
    painter->fillRect(8, height - 2, (72 + m_channels * 112), 2, colorShadow);


    //left
    painter->fillRect(0, 0, 2, height - (7), colorHilite);
    painter->fillRect(38, 0, 2, height - (16), colorShadow);

    //top
    painter->fillRect(39, 14, (m_channels * 112 - 7), 2, colorShadow);
    painter->fillRect(8, 2, (32 + m_channels * 112), 12, colorBase);
    painter->fillRect(0, 0, (72 + m_channels * 112), 2, colorHilite);

    //corners
    QString imagepath = QApplication::applicationDirPath() + QDir::separator() + "data/resources" + QDir::separator() +
        "trackerview" + QDir::separator() + "s3m_top.png";
    QImage spriteSheet(imagepath);

    QRectF sourceLeftCorner(0, 0, 8, 8);
    QRectF sourceRightCorner(8, 0, 8, 8);
    QRectF targetLeftCorner(0, height - 8, 8, 8);
    QRectF targetRightCorner((72 + m_channels * 112), 0, 8, 8);
    painter->drawImage(targetLeftCorner, spriteSheet, sourceLeftCorner);
    painter->drawImage(targetRightCorner, spriteSheet, sourceRightCorner);
}

void ScreamTracker3PatternView::paintBelow(QPainter* painter, int height, int currentRow)
{
    QColor colorBase(164, 144, 84);
    QColor colorHilite(252, 220, 132);
    QColor colorShadow(80, 68, 40);
    //main
    painter->fillRect(40, (height / 2) - 7, (40 + m_channels * 112), 8, m_colorCurrentRowBackground);

    //left
    painter->fillRect(2, 0, 38, height - (7), colorBase);
}

ScreamTracker3PatternView::~ScreamTracker3PatternView()
{
}

QString ScreamTracker3PatternView::note(BaseRow* row)
{
    if (row->note == 129) return "$$\"";
    return AbstractPatternView::note(row);
}

QString ScreamTracker3PatternView::effect(BaseRow* row)
{
    QString effectStr;
    if (row->effect < 1) return m_emptyEffect;
    switch (row->effect + 1)
    {
    case 164: effectStr = "A";
        break;
    case 11: effectStr = "D";
        break;
    case 4: effectStr = "G";
        break;
    case 173: effectStr = "U";
        break;
    case 3: effectStr = "E";
        break;
    case 2: effectStr = "F";
        break;
    case 14: effectStr = "C";
        break;
    case 15: effectStr = "S";
        break;
    case 23: effectStr = "S";
        break;
    case 7: effectStr = "K";
        break;
    case 5: effectStr = "H";
        break;
    case 10: effectStr = "O";
        break;
    case 172: effectStr = "T";
        break;
    case 28: effectStr = "Q";
        break;
    case 6: effectStr = "L";
        break;
    case 12: effectStr = "B";
        break;
    case 30: effectStr = "I";
        break;
    case 8: effectStr = "R";
        break;
    case 17: effectStr = "V";
        break;

    //                            case  1: effect = "J";break;
    //
    //
    //                            case 21: effect = "M";break;
    //                            case 22: effect = "N";break;
    //
    //                            case 29: effect = "P";break;
    //
    //
    //
    //
    //                            case 26: effect = "U";break;

    //
    //                            case 24: effect = "W";break;
    //                            case  9: effect = "X";break;
    //                            case 27: effect = "Y";break;
    //                            case 31: effect = "Z";break;

    default: effectStr = "?";
        break;
    }
    return effectStr;
}
