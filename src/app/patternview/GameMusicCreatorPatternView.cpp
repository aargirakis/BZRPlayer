#include "GameMusicCreatorPatternView.h"

#include <mainwindow.h>
#include <QApplication>
#include <QDir>

GameMusicCreatorPatternView::GameMusicCreatorPatternView(Tracker* parent, unsigned int channels)
    : AbstractPatternView(parent, channels)
{
    octaveOffset = 48;
    m_font = QFont("Game Music Creator");
    m_bitmapFont = BitmapFont("Game Music Creator");

    m_font.setPixelSize(8);
    m_fontWidth = 8;
    m_fontHeight = 8;
    rowNumberOffset = 0;
    m_emptyInstrument = "0";
    instrumentPad = false;
    m_colorCurrentRowBackground = QColor(99, 85, 66);
    m_colorCurrentRowForeground = QColor(189, 170, 0);
    m_colorDefault = QColor(0, 101, 239);
    m_ColorRowNumber = m_ColorInstrument = m_ColorEffect = m_ColorParameter = m_ColorVolume = m_colorEmpty =
        QColor(0, 16, 189);
    m_SeparatorRowNumber = m_SeparatorChannel = " ";
    m_RowLength = 40;
    m_xOffsetRow = 8;

    m_xChannelStart = 30;
    m_channelWidth = 60;
    m_channelLastWidth = 91;
    m_channelxSpace = 4;

    m_bottomFrameHeight = 8;
    m_topHeight = 27;
    m_renderTop = true;
    m_renderVUMeter = false;
}

void GameMusicCreatorPatternView::paintAbove(QPainter* painter, int height, int currentRow)
{
    QString imagepath = dataPath + RESOURCES_DIR + QDir::separator() +
        "trackerview" + QDir::separator() + "gmc_top.png";
    QImage spriteSheet(imagepath);
    //bottom border
    QRectF sourceBottomBar(0, 27, 320, 8);
    QRectF targetBottomBar(0, height - 8, 320, 8);
    painter->drawImage(targetBottomBar, spriteSheet, sourceBottomBar);
}

void GameMusicCreatorPatternView::paintBelow(QPainter* painter, int height, int currentRow)
{
    //left, right and channel borders below
    QColor colorBrown(99, 85, 66);
    QPen pen(colorBrown);
    pen.setWidth(1);
    painter->setPen(pen);
    painter->drawLine(1, 0, 1, height);
    painter->drawLine((7), 0, (7), height);
    painter->drawLine((314), 0, (314), height);
    painter->drawLine((320), 0, (320), height);
    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        painter->drawLine((26 + chan * 64), 0, (25 + chan * 64), height);
        painter->drawLine((31 + chan * 64), 0, (30 + chan * 64), height);
    }
    //current row marker
    QColor colorHilite(66, 48, 49);
    painter->fillRect((0), (height / 2) - 8, 320, 7, m_colorCurrentRowBackground);
    pen.setColor(colorHilite);
    painter->setPen(pen);
    painter->drawLine(0, (height / 2) - 9, 320, (height / 2) - 9);
    painter->drawLine(0, (height / 2) - 1, 320, height / 2 - 1);

    //left border
    QString imagepath = dataPath + RESOURCES_DIR + QDir::separator() +
        "trackerview" + QDir::separator() + "gmc_top.png";
    QImage spriteSheet(imagepath);
    QRectF sourceLeftBar(1, 26, 5, 1);
    QRectF sourceRightBar(314, 26, 5, 1);
    QRectF targetLeftBar(1, 0, 5, height);
    painter->drawImage(targetLeftBar, spriteSheet, sourceLeftBar);
    //right border
    QRectF targetRightBar(314, 0, 5, height);
    painter->drawImage(targetRightBar, spriteSheet, sourceRightBar);

    //channel borders
    QRectF sourceChannelBar(26, 26, 4, 1);
    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        QRectF targetChannelBar((26) + chan * 64, 0, 4, height);
        painter->drawImage(targetChannelBar, spriteSheet, sourceChannelBar);
    }
}
void::GameMusicCreatorPatternView::paintTop(QPainter* painter,Info* info, unsigned int m_currentPattern, unsigned int m_currentPosition, unsigned int m_currentSpeed, unsigned int m_currentBPM, unsigned int m_currentRow)
{
    m_height = 27;
    int left = 0;

    QRectF source(0, 0, 320, 27);
    QRectF target(left, 0, 320, 27);

    QString imagepath = dataPath + RESOURCES_DIR +
                        QDir::separator() + "trackerview" + QDir::separator() + "gmc_top.png";
    QImage imageTop(imagepath);
    painter->drawImage(target, imageTop, source);

    painter->setPen(QColor(173, 138, 115));
    drawText(QString("%1").arg(m_currentPosition, 2, 10, QChar('0')), painter, left + 88, 14, infoFont());
    drawText(QString("%1").arg(m_currentPattern, 2, 10, QChar('0')), painter, left + 194, 14, infoFont());
    drawText(QString("%1").arg(info->numOrders, 2, 10, QChar('0')), painter, left + 300, 14, infoFont());
}
GameMusicCreatorPatternView::~GameMusicCreatorPatternView()
{
}
