#include "Composer669PatternView.h"

#include <mainwindow.h>
#include <QApplication>
#include <QDir>

Composer669PatternView::Composer669PatternView(Tracker* parent, unsigned int channels)
    : AbstractPatternView(parent, channels)
{
    m_font.setPixelSize(8);
    m_fontWidth = 8;
    octaveOffset = 48;
    instrumentOffset = -1;
    rowNumberOffset = 0;
    rowNumberHex = true;
    volumeEnabled = true;
    volumePad = false;
    volumeHex = true;
    parameterPad = false;
    m_colorDefault = QColor(130, 190, 223);
    m_ColorVolume = QColor(130, 190, 223);
    m_ColorRowNumber = QColor(0, 125, 117);
    m_ColorInstrument = QColor(105, 138, 153);
    m_ColorEffect = QColor(130, 0, 255);
    m_ColorParameter = QColor(97, 0, 195);
    m_emptyVolume = m_emptyParameter = m_emptyEffect = "\"";
    m_emptyInstrument = "\"\"";
    m_emptyNote = "\"\"\"";
    m_colorCurrentRowBackground = QColor(44, 89, 134);
    m_colorCurrentRowForeground = QColor(0, 211, 255);
    m_RowHighlightBackgroundFrequency = 4;
    m_ColorRowHighlightBackground = QColor(40, 52, 65);
    m_colorEmpty = QColor(105, 138, 154);
    m_yOffsetRowHighlight = -7;
    m_highlightBackgroundOffset = 64;

    m_SeparatorRowNumber = "'''''";
    m_RowStart = m_RowEnd = m_SeparatorChannel = "'";

    m_RowLength = 80;

    m_xChannelStart = 64;
    m_channelWidth = 64;
    m_channelxSpace = 8;

    m_bottomFrameHeight = 16;
    m_topHeight = 16;
}

void Composer669PatternView::paintAbove(QPainter* painter, int height, int currentRow)
{
    //Always 8 channels
    QColor colorBlue(28, 56, 85);
    QColor colorLightBlue(44, 89, 134);
    QColor colorPurple(65, 0, 130);

    //left
    painter->fillRect(0, 0, 8, height, colorBlue);
    painter->fillRect(24, 0, 8, height, colorBlue);
    painter->fillRect(32, 0, 24, height, colorPurple);
    for (int chan = 0; chan < 8; chan++)
    {
        painter->fillRect((56 + chan * 72), 0, 8, height, colorBlue);
    }

    //right
    painter->fillRect(632, 0, 8, height, colorBlue);
    //bottom
    painter->fillRect(0, height - 16, 640, 8, colorBlue);
    painter->fillRect(0, height - 8, 640, 8, colorLightBlue);
    //top
    painter->fillRect(0, 0, 640, 8, colorLightBlue);
    painter->fillRect(0, 8, 640, 8, colorBlue);
    QString imagepath = dataPath + RESOURCES_DIR + QDir::separator() + "trackerview" + QDir::separator() +
                        "composer669_top.png";

    //symbol
    QImage spriteSheet(imagepath);
    QRectF source(0, 0, 23, 7);
    QRectF target(32, (height / 2) - 7, 23, 7);
    painter->drawImage(target, spriteSheet, source);
}

void Composer669PatternView::paintBelow(QPainter* painter, int height, int currentRow)
{
    //main
    painter->fillRect(64, (height / 2) - 7, 568, 8, m_colorCurrentRowBackground);
}

Composer669PatternView::~Composer669PatternView()
{
}

QString Composer669PatternView::volume(BaseRow* row)
{
    if (note(row) == m_emptyNote) return m_emptyVolume;
    return AbstractPatternView::volume(row);
}

QString Composer669PatternView::note(BaseRow* row)
{
    int note = row->note;
    switch (note)
    {
    case 37: return "C'0";
    case 38: return "C#0";
    case 39: return "D'0";
    case 40: return "D#0";
    case 41: return "E'0";
    case 42: return "F'0";
    case 43: return "F#0";
    case 44: return "G'0";
    case 45: return "G#0";
    case 46: return "A'0";
    case 47: return "A#0";
    }


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
        return QString(NOTES[note])[0] + QString("'") + QString(NOTES[note])[2];
    }
}

QString Composer669PatternView::effect(BaseRow* row)
{
    QString effectStr;
    switch (row->effect)
    {
    case 121: effectStr = "a";
        break;
    case 120: effectStr = "b";
        break;
    case 122: effectStr = "c";
        break;
    case 166: effectStr = "d";
        break;
    case 123: effectStr = "e";
        break;
    case 126: effectStr = "f";
        break;
    default: effectStr = m_emptyEffect;
        break;
    }
    return effectStr;
}

QString Composer669PatternView::parameter(BaseRow* row)
{
    int parameter = row->param;
    if (row->effect == 0 && parameter == 0) return m_emptyParameter;
    if (row->effect == 166)
    {
        switch (parameter)
        {
        case 0x90: parameter = 1;
            break;
        case 0xa0: parameter = 2;
            break;
        case 0xb0: parameter = 3;
            break;
        case 0xc0: parameter = 4;
            break;
        case 0xd0: parameter = 5;
            break;
        case 0xe0: parameter = 6;
            break;
        case 0xf0: parameter = 7;
            break;
        case 0x00: parameter = 8;
            break;
        case 0x10: parameter = 9;
            break;
        case 0x20: parameter = 10;
            break;
        case 0x30: parameter = 11;
            break;
        case 0x40: parameter = 12;
            break;
        case 0x50: parameter = 13;
            break;
        case 0x60: parameter = 14;
            break;
        case 0x70: parameter = 15;
            break;
        case 0x80: parameter = 0;
            break;
        }
    }
    int base = parameterHex ? 16 : 10;

    QString parameterStr = QString::number(parameter, base).toUpper();
    parameterStr = parameterPad && parameterStr.length() == 1 ? "0" + parameterStr : parameterStr;
    return parameterStr;
}
