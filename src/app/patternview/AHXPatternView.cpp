#include <QDir>
#include "AHXPatternView.h"
#include "visualizers/tracker.h"
#include "mainwindow.h"

AHXPatternView::AHXPatternView(Tracker* parent, unsigned int channels, int scale)
    : AbstractPatternView(parent, channels, scale)
{
    instrumentPad = true;
    instrumentHex = false;
    m_font = QFont("AHX");
    m_font.setPixelSize(8);
    m_fontWidth = 8;

    m_bitmapFont = BitmapFont("AHX");
    m_fontWidth = 8;
    m_fontHeight = 8;
    m_bitmapFont3 = BitmapFont("AHX Thin");

    rowNumberOffset = 0;
    m_colorDefault = m_ColorEffect = m_ColorInstrument = m_ColorParameter = m_ColorRowNumber = m_ColorVolume =
        m_colorEmpty = QColor(206, 231, 231);
    m_colorCurrentRowForeground = QColor(255, 255, 255);
    m_colorCurrentRowBackground = QColor(82, 89, 99);
    m_SeparatorRowNumber = " ";
    m_RowEnd = m_SeparatorChannel = " ";
    m_RowLength = 40;
    m_xOffsetRow = 14;
    m_xOffsetSeparatorRowNumber = -3;

    //for dimming channels
    m_xChannelStart = 32;
    m_channelWidth = 70;
    m_channelLastWidth = m_channelWidth;
    m_channelxSpace = 2;
    m_topHeight = 44;
    m_bottomFrameHeight = 2;
}

BitmapFont AHXPatternView::infoFont()
{
    return m_bitmapFont3;
}

void AHXPatternView::paintAbove(QPainter* painter, int height, int currentRow)
{
    QColor colorHilite = QColor(255, 255, 255);
    QColor colorShadow = QColor(82, 89, 99);
    QPen pen(colorHilite);
    pen.setWidth(1);
    //top
    pen.setColor(colorShadow);
    painter->setPen(pen);
    painter->drawLine(14, 43, 319, 43);

    //bottom
    pen.setColor(colorHilite);
    painter->setPen(pen);
    painter->drawLine(13, height - 2, 319, height - 2);
    pen.setColor(colorShadow);
    painter->setPen(pen);
    painter->drawLine(14, height - 1, 319, height - 1);


    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        //channel dividers
        pen.setColor(colorHilite);
        painter->setPen(pen);
        painter->drawLine((31 + chan * 72), 43, (31 + chan * 72), height - 2);
        pen.setColor(colorShadow);
        painter->setPen(pen);
        painter->drawLine((32 + chan * 72), 43, (32 + chan * 72), height - 3);
        painter->setPen(pen);
    }

    //top
    pen.setColor(colorHilite);
    painter->setPen(pen);
    painter->drawLine(13, 42, 319, 42);


    //right border
    pen.setColor(colorHilite);
    painter->setPen(pen);
    painter->drawLine((319), 43, (319), height - 3);
    pen.setColor(colorShadow);
    painter->setPen(pen);
    painter->drawLine((320), 0, (320), height - 1);
}

void AHXPatternView::paintTop(QPainter* painter,Info* info, unsigned int m_currentPattern, unsigned int m_currentPosition, unsigned int m_currentSpeed, unsigned int m_currentBPM, unsigned int m_currentRow)
{
    //TODO
    //some characters missing in font, copyright symbol, maybe others

    m_height = 42;
    int top = 0;
    int left = 0;
    QColor colorBase(140, 134, 146);
    QColor colorHilite(255, 255, 255);
    QColor colorShadow(82, 89, 99);


    int imageWidth = 320;
    int imageHeight = 42;

    QRectF source(0, 0, imageWidth, imageHeight);
    QRectF target(left, top, imageWidth, imageHeight);

    QString imagepath = dataPath + RESOURCES_DIR +
                        QDir::separator() + "trackerview" + QDir::separator() + "ahx_top.png";
    QImage imageTop(imagepath);
    painter->drawImage(target, imageTop, source);


    QPen pen(colorHilite);
    pen.setColor(colorHilite);
    painter->setPen(pen);

    for (int i = 0; i < info->modTrackPositions.size(); i++)
    {
        drawText(QString("%1").arg(info->modTrackPositions[i], 3, 10, QChar('0')), painter,
                 left + (32) + (i * 72), top + (40),infoFont());
    }

    drawText(QString("%1").arg(m_currentPosition, 3, 10, QChar('0')), painter, left + 26, top + (11),infoFont());
    drawText(QString("%1").arg(info->numOrders, 3, 10, QChar('0')), painter, left + 76, top + (11),infoFont());
    drawText(QString("%1").arg(info->modPatternRestart, 3, 10, QChar('0')), painter, left + 126,top + (11),infoFont());
    drawText(QString("%1").arg(info->modPatternRows, 3, 10, QChar('0')), painter, left + 176, top + (11),infoFont());
    drawText(QString("%1").arg(info->numSubsongs, 3, 10, QChar('0')), painter, left + 219, top + (11),infoFont());
    drawText("#blablbla", painter, left + (33), top + (24),infoFont());
}
void AHXPatternView::paintBelow(QPainter* painter, int height, int currentRow)
{
    //current row marker
    QColor colorBase = QColor(140, 134, 156);
    QColor colorHilite = QColor(255, 255, 255);
    QColor colorShadow = QColor(82, 89, 99);
    QPen pen(colorBase);

    painter->fillRect(0, 0, 13, (height), colorBase);

    pen.setWidth(1);
    painter->setPen(pen);
    painter->fillRect((14), (height / 2) - 8, 305, 7, m_colorCurrentRowBackground);
    painter->drawLine((14), (height / 2) - 9, (319), ((height / 2) - 9));
    painter->drawLine((14), (height / 2) - 1, (319), ((height / 2) - 1));

    //left border
    pen.setColor(colorHilite);
    painter->setPen(pen);
    painter->drawLine((13), 0, (13), height - 3);
    pen.setColor(colorShadow);
    painter->setPen(pen);
    painter->drawLine((14), 0, (14), height - 1);
    painter->setPen(pen);
}



AHXPatternView::~AHXPatternView()
{
}
