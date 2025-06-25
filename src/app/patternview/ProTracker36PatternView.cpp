#include "ProTracker36PatternView.h"

#include <mainwindow.h>
#include <QApplication>
#include <QDir>

ProTracker36PatternView::ProTracker36PatternView(Tracker* parent, unsigned int channels)
    : AbstractPatternView(parent, channels)
{
    rowNumberOffset = 0;
    octaveOffset = 48;
    m_font = QFont("Protracker 3.61");
    m_font.setPixelSize(16);
    m_fontWidth = 12;
    m_font.setStyleStrategy(QFont::NoAntialias);
    m_renderTop = true;
    m_renderVUMeter = true;

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

    m_vumeterWidth = 12;
    m_vumeterHeight = 94;
    m_vumeterLeftOffset = 90;
    m_vumeterHilightWidth = 2;
    m_vumeterOffset = 120;
    m_vumeterTopOffset = -18;



    setupVUMeters();
//main color
    m_linearGrad.setColorAt(0, QColor(0, 0, 0).rgb()); //black
    m_linearGrad.setColorAt(0.01999, QColor(0, 0, 0).rgb()); //black
    m_linearGrad.setColorAt(0.02, QColor(75, 0, 0).rgb()); //red
    m_linearGrad.setColorAt(0.07, QColor(136, 17, 0).rgb()); //red
    m_linearGrad.setColorAt(0.12, QColor(170, 34, 0).rgb()); //red
    m_linearGrad.setColorAt(0.22, QColor(238, 102, 0).rgb()); //red
    m_linearGrad.setColorAt(0.39, QColor(255, 238, 0).rgb()); //yellow
    m_linearGrad.setColorAt(0.48, QColor(238, 255, 0).rgb()); //yellow
    m_linearGrad.setColorAt(0.64, QColor(153, 238, 0).rgb()); // green
    m_linearGrad.setColorAt(1, QColor(17, 85, 0).rgb()); // green


    //hilight color (left)
    m_linearGradHiLite.setColorAt(0, QColor(0, 0, 0).rgb()); //black
    m_linearGradHiLite.setColorAt(0.01999, QColor(0, 0, 0).rgb()); //black
    m_linearGradHiLite.setColorAt(0.02, QColor(136, 51, 51).rgb()); //red
    m_linearGradHiLite.setColorAt(0.07, QColor(187, 68, 51).rgb()); //red
    m_linearGradHiLite.setColorAt(0.12, QColor(221, 85, 51).rgb()); //red
    m_linearGradHiLite.setColorAt(0.22, QColor(255, 153, 51).rgb()); //red
    m_linearGradHiLite.setColorAt(0.39, QColor(255, 255, 51).rgb()); //yellow
    m_linearGradHiLite.setColorAt(0.48, QColor(255, 255, 51).rgb()); //yellow
    m_linearGradHiLite.setColorAt(0.64, QColor(204, 255, 51).rgb()); // green
    m_linearGradHiLite.setColorAt(1, QColor(68, 136, 51).rgb()); // green

    //dark color (right)
    m_linearGradDark.setColorAt(0, QColor(0, 0, 0).rgb()); //black
    m_linearGradDark.setColorAt(0.01999, QColor(0, 0, 0).rgb()); //black
    m_linearGradDark.setColorAt(0.02, QColor(34, 0, 0).rgb()); //red
    m_linearGradDark.setColorAt(0.07, QColor(85, 0, 0).rgb()); //red
    m_linearGradDark.setColorAt(0.12, QColor(119, 0, 0).rgb()); //red
    m_linearGradDark.setColorAt(0.22, QColor(187, 51, 0).rgb()); //red
    m_linearGradDark.setColorAt(0.39, QColor(204, 170, 0).rgb()); //yellow
    m_linearGradDark.setColorAt(0.48, QColor(187, 204, 0).rgb()); //yellow
    m_linearGradDark.setColorAt(0.64, QColor(102, 187, 51).rgb()); // green
    m_linearGradDark.setColorAt(1, QColor(0, 34, 0).rgb()); // green

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

    QString imagepath = dataPath + RESOURCES_DIR + QDir::separator() +
        "trackerview" + QDir::separator() + "protracker361_top.png";
    QImage spriteSheet(imagepath);

    QRectF sourceScrollBarBg(0, 0, 13, 4);


    int numberOfScrollBarBgPieces = ((this->height()-topHeight())/4)-1;

    for (int i = 0; i < numberOfScrollBarBgPieces; i++)
    {
        QRectF targetLeftBar((520), (46) + i * 4, 13, 4);
        painter->drawImage(targetLeftBar, spriteSheet, sourceScrollBarBg);
    }

    painter->fillRect((518), 42, 1, height-topHeight()-1, colorHilite);
    painter->fillRect((519), 42, 1, height-topHeight()-1, colorShadow);
    painter->fillRect((533), 42, 1, height-topHeight()-1, colorHilite);
    painter->fillRect((534), 42, 1, height-topHeight()-1, colorShadow);
    painter->fillRect((535), 42, 1, height-topHeight()-1, colorHilite);

    cout << "height: " << height << "\n";
    fflush(stdout);

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
        painter->fillRect((38) + chan * 120, 0, 2, height, colorHilite);
        painter->fillRect((43) + chan * 120, 0, 2, height, colorShadow);

        painter->fillRect((83) + chan * 120, 0, 1, height, colorHilite);
        painter->fillRect((84) + chan * 120, 2, 1, height, colorShadow);
        painter->fillRect((107) + chan * 120, 0, 1, height, colorHilite);
        painter->fillRect((108) + chan * 120, 2, 1, height, colorShadow);
        painter->fillRect((131) + chan * 120, 0, 1, height, colorHilite);
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

void::ProTracker36PatternView::paintTop(QPainter* painter,Info* info, unsigned int m_currentPattern, unsigned int m_currentPosition, unsigned int m_currentSpeed, unsigned int m_currentBPM, unsigned int m_currentRow)
{
    m_topHeight = 41;
    QColor colorBase(170, 170, 170);
    QColor colorHilite(255, 255, 255);
    QColor colorShadow(51, 0, 0);
    QColor colorBlue(0, 0, 187);
    int top = 0;
    int left = 0;
    QRect rectBg(left, 0, 544, m_topHeight);
    painter->fillRect(rectBg, colorBase);

    painter->setPen(colorShadow);

    drawText("SONGNAME", painter, left + (4), top + (18), infoFont());
    painter->setPen(colorHilite);
    drawText("SONGNAME", painter, left + (3), top + (16), infoFont());
    painter->setPen(QColor(colorShadow));
    painter->setPen(colorBlue);

    drawText(QString("%1").arg(info->title.c_str(), -20, QChar('_')), painter, left + (65), top + (18), infoFont2());
    painter->setPen(colorShadow);

    drawText("POSITION", painter, left + (234), top + (18), infoFont());
    painter->setPen(colorHilite);
    drawText("POSITION", painter, left + (233), top + (16), infoFont());
    painter->setPen(colorShadow);
    painter->setPen(QColor(colorShadow));
    painter->setPen(colorBlue);

    drawText(QString("%1").arg(m_currentPosition, 4, 10, QChar('0')), painter, left + (296), top + (18), infoFont2());
    painter->setPen(colorShadow);

    drawText("PATTERN", painter, left + (339), top + (18), infoFont());
    painter->setPen(colorHilite);
    drawText("PATTERN", painter, left + (338), top + (16), infoFont());
    painter->setPen(colorBlue);

    drawText(QString("%1").arg(m_currentPattern, 4, 10, QChar('0')), painter, left + (401), top + (18), infoFont2());
    painter->setPen(colorShadow);

    drawText("LENGTH", painter, left + (444), top + (18), infoFont());
    painter->setPen(colorHilite);
    drawText("LENGTH", painter, left + (443), top + (16), infoFont());
    painter->setPen(QColor(colorShadow));
    painter->setPen(colorBlue);

    drawText(QString("%1").arg(info->numOrders, 4, 10, QChar('0')), painter, left + (506), top + (18), infoFont2());

    painter->setPen(colorShadow);
    drawText("POS", painter, left + (13), top + (38), infoFont());
    painter->setPen(colorHilite);
    drawText("POS", painter, left + (12), top + (36), infoFont());
    for (int i = 0; i < 4; i++)
    {
        painter->setPen(QColor(colorShadow));
        drawText("TRACK #" + QString::number(i + 1), painter, left + (69 + (120 * i)), top + (38), infoFont(), 1);
        painter->setPen(colorHilite);
        drawText("TRACK #" + QString::number(i + 1), painter, left + (68 + (120 * i)), top + (36), infoFont(), 1);
    }

    painter->fillRect(left, 0, 1, m_topHeight, colorHilite);
    painter->fillRect(left + 543, 0, 1, m_topHeight, colorShadow);
    painter->fillRect(left, 0, 543, 2, colorHilite);

    painter->fillRect(left, 20, 543, 2, colorShadow);
    painter->fillRect(left, 22, 543, 2, colorHilite);

    //bevels
    painter->fillRect(left + 228, 2, 1, 18, colorShadow);
    painter->fillRect(left + 229, 2, 1, 18, colorHilite);
    painter->fillRect(left + 333, 2, 1, 18, colorShadow);
    painter->fillRect(left + 334, 2, 1, 18, colorHilite);
    painter->fillRect(left + 438, 2, 1, 18, colorShadow);
    painter->fillRect(left + 439, 2, 1, 18, colorHilite);
    //bevels antialiasing
    painter->fillRect(left, 20, 1, 2, colorBase);
    painter->fillRect(left + 228, 0, 1, 2, colorBase);
    painter->fillRect(left + 229, 20, 1, 2, colorBase);
    painter->fillRect(left + 333, 0, 1, 2, colorBase);
    painter->fillRect(left + 334, 20, 1, 2, colorBase);
    painter->fillRect(left + 438, 0, 1, 2, colorBase);
    painter->fillRect(left + 439, 20, 1, 2, colorBase);
    painter->fillRect(left + 543, 0, 1, 2, colorBase);
    painter->fillRect(left + 543, 22, 1, 2, colorBase);
}
ProTracker36PatternView::~ProTracker36PatternView()
{
}
