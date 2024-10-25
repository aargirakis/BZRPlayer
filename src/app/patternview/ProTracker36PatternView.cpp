#include "ProTracker36PatternView.h"
#include <QApplication>
#include <QDir>

ProTracker36PatternView::ProTracker36PatternView(Tracker* parent, unsigned int channels, int scale)
    : AbstractPatternView(parent, channels, scale)
{
    rowNumberOffset = 0;
    octaveOffset = 48;
    m_font = QFont("Protracker 3.61");
    m_font.setPixelSize(16);
    m_fontWidth = 12;
    m_font.setStyleStrategy(QFont::NoAntialias);

    m_bitmapFont = BitmapFont("Protracker 3.61");
    m_fontWidth = 12;
    m_fontHeight = 16;
    m_bitmapFont3 = BitmapFont("Protracker 3.61 GUI 2");
    m_bitmapFont4 = BitmapFont("Protracker 3.61 GUI");

    m_colorDefault = m_ColorRowNumber = m_ColorInstrument = m_ColorEffect = m_ColorParameter = m_ColorEffect2 =
        m_ColorParameter2 = m_ColorVolume = m_colorEmpty = QColor(51, 221, 255);
    m_colorCurrentRowForeground = QColor(0, 0, 187);
    m_emptyNote = " - ";
    m_RowEnd = m_SeparatorRowNumber = m_SeparatorChannel = " ";
    m_RowLength = 46;
    m_xOffsetRow = 12;
    effectPad = true;
    m_emptyEffect = "00";
    m_xChannelStart = 45;
    m_channelWidth = 113;
    m_channelxSpace = 7;

    m_bottomFrameHeight = 4;
    m_topHeight = 42;
    m_width = 640;
    m_height = 512;
}

BitmapFont ProTracker36PatternView::infoFont()
{
    return m_bitmapFont3;
}

BitmapFont ProTracker36PatternView::infoFont2()
{
    return m_bitmapFont4;
}

void ProTracker36PatternView::paintAbove(QPainter* painter, int height, int currentRow)
{
    QColor colorBase(170, 170, 170);
    QColor colorHilite(255, 255, 255);
    QColor colorShadow(51, 0, 0);

    //scrollbar

    QString imagepath = QApplication::applicationDirPath() + QDir::separator() + "data/resources" + QDir::separator() +
        "trackerview" + QDir::separator() + "protracker361_top.png";
    QImage spriteSheet(imagepath);

    QRectF sourceScrollBarBg(0, 0, 13, 4);

    int numberOfScrollBarBgPieces = ((height - 14)) + 1;

    for (int i = 0; i < numberOfScrollBarBgPieces; i++)
    {
        QRectF targetLeftBar((520), (46) + i * 4, 13, 4);
        painter->drawImage(targetLeftBar, spriteSheet, sourceScrollBarBg);
    }

    painter->fillRect((518), 42, 1, height - (1), colorHilite);
    painter->fillRect((519), 42, 1, height - (1), colorShadow);
    painter->fillRect((533), 42, 1, height - (1), colorHilite);
    painter->fillRect((534), 42, 1, height - (1), colorShadow);
    painter->fillRect((535), 42, 1, height - (1), colorHilite);

    float currentRowPos = currentRow / 64.0;
    int yPos = (currentRowPos * (height - 88)) + 44;
    QRectF scrollBarSource(0, 4, 10, 48);
    QRectF scrollBarTarget((522), yPos, 10, 48);
    painter->drawImage(scrollBarTarget, spriteSheet, scrollBarSource);


    //bottom
    painter->fillRect(8, (height) - 4, 535, 4, colorBase);

    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        int last = 0;
        if (chan == m_channels - 1)
        {
            last = 17;
        }
        //top shadow
        painter->fillRect((43 + chan * 120), 40, (115 + last), 2, colorShadow);
        if (chan == m_channels - 1)
        {
            last = 16;
        }
        //bottom
        painter->fillRect((45 + chan * 120), (height) - 4, (115 + last), 2, colorHilite);
    }
    //main row number
    painter->fillRect((10), (height / 2) - 2, 29, 2, colorShadow);
    painter->fillRect((10), (height / 2) - 18, 28, 2, colorHilite);
    painter->fillRect((9), (height / 2) - 18, 1, 2, colorBase);

    //row number bottom
    painter->fillRect((10), (height) - 4, 30, 2, colorHilite);

    //row number top
    painter->fillRect((10), 40, 28, 2, colorShadow);

    //antialias far right
    painter->fillRect(517, (height / 2) - 18, 1, 2, colorBase);
    //shadow far right
    painter->fillRect(517, (height / 2) - 16, 1, 14, colorShadow);


    //scrollbar top
    painter->fillRect((519), 42, 14, 2, colorHilite);
    painter->fillRect((519), 44, 14, 2, colorShadow);
    painter->fillRect((534), 42, 1, 2, colorBase);

    //scrollbar bottom
    painter->fillRect((520), height - (8), 13, 2, colorHilite);
    painter->fillRect((519), height - (6), 15, 2, colorShadow);
    painter->fillRect((518), height - (6), 1, 2, colorBase);
}

void ProTracker36PatternView::paintBelow(QPainter* painter, int height, int currentRow)
{
    QColor colorBase(170, 170, 170);
    QColor colorHilite(255, 255, 255);
    QColor colorShadow(51, 0, 0);
    QPen pen(colorBase);
    pen.setWidth(1);
    painter->setPen(pen);

    //left border
    painter->fillRect(0, 0, 8, height, colorBase);
    painter->fillRect(0, 0, 1, height - 2, colorHilite);
    painter->fillRect(8, 0, 2, height, colorShadow);

    //right border
    painter->fillRect(535, 0, 8, height, colorBase);
    painter->fillRect(543, 0, 1, height, colorShadow);

    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        //channel dividers
        painter->fillRect((38) + chan * 120, 0, 5, height, colorBase);
        painter->fillRect((38) + chan * 120, 42, 2, height, colorHilite);
        painter->fillRect((43) + chan * 120, 2, 2, height, colorShadow);

        painter->fillRect((83) + chan * 120, 42, 1, height, colorHilite);
        painter->fillRect((84) + chan * 120, 2, 1, height, colorShadow);
        painter->fillRect((107) + chan * 120, 42, 1, height, colorHilite);
        painter->fillRect((108) + chan * 120, 2, 1, height, colorShadow);
        painter->fillRect((131) + chan * 120, 42, 1, height, colorHilite);
        painter->fillRect((132) + chan * 120, 2, 1, height, colorShadow);


        //main hilite and shadow
        painter->fillRect((45) + chan * 120, (height / 2) - 18, 113, 2, colorHilite);
        painter->fillRect((45) + chan * 120, (height / 2) - 2, 114, 2, colorShadow);
        //antialias
        painter->fillRect((44) + chan * 120, (height / 2) - 18, 1, 2, colorBase);
        painter->fillRect((38) + chan * 120, (height / 2) - 2, 1, 2, colorShadow);
        painter->fillRect((39) + chan * 120, (height / 2) - 2, 1, 2, colorBase);
        painter->fillRect((38) + chan * 120, (height / 2), 1, 2, colorBase);

        //vumeter base
        painter->fillRect((90) + chan * 120, (height / 2) - 18, 2, 2, QColor(66, 129, 48));
        painter->fillRect((92) + chan * 120, (height / 2) - 18, 8, 2, QColor(17, 80, 0));
        painter->fillRect((100) + chan * 120, (height / 2) - 18, 2, 2, QColor(0, 31, 0));
    }
    //main
    painter->fillRect((8), (height / 2) - 16, 509, 14, colorBase);
}

ProTracker36PatternView::~ProTracker36PatternView()
{
}
