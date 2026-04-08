#include <QDir>
#include "DigiBooster17PatternView.h"
#include "mainwindow.h"

DigiBooster17PatternView::DigiBooster17PatternView(Tracker *parent, const unsigned int channels) : AbstractPatternView(
    parent, channels) {
    octaveOffset = 48;
    rowNumberOffset = 0;
    instrumentHex = false;

    m_font2 = QFont("Digi Booster 1.7 Double Height");
    m_font2.setPixelSize(29);
    m_font = QFont("Digi Booster 1.7");
    m_font.setPixelSize(18);
    m_fontWidth = 8;

    m_bitmapFont = BitmapFont("Digi Booster 1.7");
    m_bitmapFont2 = BitmapFont("Digi Booster 1.7 Double Height");
    m_bitmapFont3 = BitmapFont("Digi Booster 1.7 Big");
    m_fontWidth = 8;
    m_fontHeight = 18;

    m_colorCurrentRowBackground = QColor(82, 85, 99);
    m_colorCurrentRowForeground = QColor(222, 186, 140);

    m_colorDefault = m_ColorRowNumber = m_ColorInstrument = m_ColorEffect = m_ColorParameter = m_ColorEffect2 =
                                                                                m_ColorParameter2 =
                                                                                m_ColorVolume = m_colorEmpty = QColor(
                                                                                        173, 170, 173);

    m_RowEnd = m_SeparatorRowNumber = m_SeparatorChannel = "'";
    m_xOffsetRow = 8;
    m_yOffsetCurrentRowAfter = -4;
    m_RowLength = 4 + 9 * m_channels + 4;

    m_xChannelStart = 28;
    m_channelWidth = 69;
    m_channelxSpace = 3;
    m_topHeight = 46;
    m_bottomFrameHeight = 6;
    m_renderTop = true;
}

QFont DigiBooster17PatternView::currentRowFont() {
    return m_font2;
}

BitmapFont DigiBooster17PatternView::currentRowBitmapFont() {
    return m_bitmapFont2;
}

BitmapFont DigiBooster17PatternView::infoFont() {
    return m_bitmapFont3;
}

void DigiBooster17PatternView::paintAbove(QPainter *painter, const int height, int currentRow) {
    const QString imagepath = dataPath + RESOURCES_DIR + QDir::separator() +
                              "trackerview" + QDir::separator() + "digibooster17_top.png";
    const QImage spriteSheet(imagepath);

    // right border (channel buttons)
    constexpr QRectF sourceRightBar(18, 84, 35, 174);
    const QRectF targetRightBar(602, height - 186, 35, 174);
    painter->drawImage(targetRightBar, spriteSheet, sourceRightBar);

    // left border
    constexpr QRectF sourceLeftBar(12, 84, 6, 147);

    const int numberOfLeftBarPieces = (height - 33) / 147 + 1;

    for (int i = 0; i < numberOfLeftBarPieces; i++) {
        QRectF targetLeftBar(0, 46 + i * 147, 6, 147);
        painter->drawImage(targetLeftBar, spriteSheet, sourceLeftBar);
    }

    // left bottom border
    constexpr QRectF sourceLeftBottomBar(6, 84, 6, 20);
    const QRectF targetLeftBottomBar(0, height - 26, 6, 20);
    painter->drawImage(targetLeftBottomBar, spriteSheet, sourceLeftBottomBar);

    // bottom border
    constexpr QRectF sourceBottomBar(0, 46, 640, 6);
    const QRectF targetBottomBar(0, height - 6, 640, 6);
    painter->drawImage(targetBottomBar, spriteSheet, sourceBottomBar);
}

void DigiBooster17PatternView::paintBelow(QPainter *painter, const int height, int currentRow) {
    constexpr QColor colorHilite(82, 85, 99);
    constexpr QColor colorShadow(49, 48, 66);

    QPen pen(colorShadow);
    pen.setWidth(1);
    painter->setPen(pen);

    for (int chan = 0; chan < m_channels; chan++) {
        // channel dividers
        pen.setColor(colorShadow);
        painter->setPen(pen);
        painter->drawLine(26 + chan * 72, 0, 26 + chan * 72, height);
        pen.setColor(colorHilite);
        painter->setPen(pen);
        painter->drawLine(27 + chan * 72, 0, 27 + chan * 72, height - 4);
        pen.setColor(colorShadow);
        painter->setPen(pen);
        painter->drawLine(28 + chan * 72, 0, 28 + chan * 72, height);
    }

    const QString imagepath = dataPath + RESOURCES_DIR + QDir::separator() +
                              "trackerview" + QDir::separator() + "digibooster17_top.png";
    const QImage spriteSheet(imagepath);

    // bottom border
    constexpr QRectF sourceBar(0, 52, 596, 32);
    const QRectF targetBar(6, height / 2 - 17, 596, 32);
    painter->drawImage(targetBar, spriteSheet, sourceBar);
}

void ::DigiBooster17PatternView::paintTop(QPainter *painter, Info *info, const unsigned int m_currentPattern,
                                          const unsigned int m_currentPosition, unsigned int m_currentSpeed,
                                          unsigned int m_currentBPM, unsigned int m_currentRow) {
    constexpr QColor fontColor(222, 186, 140);

    constexpr int top = 18;
    constexpr int left = 0 + 4;
    constexpr int imageWidth = 640;
    constexpr int imageHeight = 46;

    constexpr QRectF source(0, 0, imageWidth, imageHeight);
    constexpr QRectF target(left - 4, 0, imageWidth, imageHeight);

    const auto t = this->parent();
    const QString imagepath = dataPath + RESOURCES_DIR +
                              QDir::separator() + "trackerview" + QDir::separator() + "digibooster17_top.png";
    const QImage imageTop(imagepath);
    painter->drawImage(target, imageTop, source);

    painter->setPen(QColor(0, 0, 0));
    drawText(QString("%1").arg(m_currentPosition, 4, 10, QChar('0')), painter, left + 122, top + 2, infoFont());
    painter->setPen(fontColor);
    drawText(QString("%1").arg(m_currentPosition, 4, 10, QChar('0')), painter, left + 122, top, infoFont());
    painter->setPen(QColor(0, 0, 0));
    drawText(QString("%1").arg(m_currentPattern, 4, 10, QChar('0')), painter, left + 331, top + 2, infoFont());
    painter->setPen(fontColor);
    drawText(QString("%1").arg(m_currentPattern, 4, 10, QChar('0')), painter, left + 331, top, infoFont());
    painter->setPen(QColor(0, 0, 0));
    drawText(QString("%1").arg(t->info->numOrders, 4, 10, QChar('0')), painter, left + 540, top + 2, infoFont());
    painter->setPen(fontColor);
    drawText(QString("%1").arg(t->info->numOrders, 4, 10, QChar('0')), painter, left + 540, top, infoFont());
    painter->setPen(QColor(0, 0, 0));
    drawText(QString(t->info->title.c_str()), painter, left + 72, top + 24, infoFont());
    painter->setPen(fontColor);
    drawText(QString(t->info->title.c_str()), painter, left + 72, top + 22, infoFont());
}

DigiBooster17PatternView::~DigiBooster17PatternView() {
}
