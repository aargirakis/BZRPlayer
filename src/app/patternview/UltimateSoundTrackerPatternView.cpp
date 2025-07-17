#include "UltimateSoundTrackerPatternView.h"
#include <QDir>
#include "mainwindow.h"

UltimateSoundTrackerPatternView::UltimateSoundTrackerPatternView(Tracker* parent, unsigned int channels, int scale)
    : AbstractPatternView(parent, channels, scale)
{
    rowNumberOffset = 0;
    octaveOffset = 48;
    m_font2 = QFont("UST Double Height");
    m_font2.setPixelSize(14);
    m_font = QFont("Ultimate Soundtracker");
    m_font.setPixelSize(7);

    m_bitmapFont = BitmapFont("Ultimate Soundtracker");
    m_bitmapFont2 = BitmapFont("Ultimate Soundtracker Double Height");
    m_fontWidth = 8;
    m_fontHeight = 7;

    m_fontWidth = 8;
    m_emptyInstrument = "0";
    m_SeparatorNote = "'";
    instrumentPad = false;
    m_colorCurrentRowBackground = QColor(156, 117, 82);
    m_colorDefault = m_ColorEffect = m_ColorRowNumber = m_ColorInstrument = m_ColorParameter = m_ColorVolume =
        m_colorEmpty = QColor(0, 101, 156);
    m_RowEnd = m_SeparatorRowNumber = m_SeparatorChannel = "'";
    m_RowLength = 40;
    m_xOffsetRow = 9;

    //for dimming channels
    m_xChannelStart = 30;
    m_channelWidth = 69;
    m_channelLastWidth = 71;
    m_channelxSpace = 3;

    m_ibuttonPrevSampleWidth = 9;
    m_ibuttonPrevSampleHeight = 7;
    m_ibuttonPrevSampleX = 14;
    m_ibuttonPrevSampleY = 24;
    m_ibuttonNextSampleWidth = 9;
    m_ibuttonNextSampleHeight = 7;
    m_ibuttonNextSampleX = 2;
    m_ibuttonNextSampleY = 24;
}

QFont UltimateSoundTrackerPatternView::currentRowFont()
{
    return m_font2;
}

BitmapFont UltimateSoundTrackerPatternView::currentRowBitmapFont()
{
    return m_bitmapFont2;
}

void UltimateSoundTrackerPatternView::paintAbove(QPainter* painter, int height, int currentRow)
{
    //bottom

    QColor colorBase(156, 117, 82);
    QColor colorHilite(173, 138, 99);
    QColor colorShadow(82, 48, 16);
    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        int extra = 0;
        if (chan == m_channels - 1)
        {
            extra = 2;
        }
        painter->fillRect((29) + chan * 72, (height) - 3, (70 + extra), 1, colorHilite);
        painter->fillRect((29) + chan * 72, 32, (70 + extra), 1, colorShadow);
    }
    painter->fillRect((2), 32, 25, 1, colorShadow);
    painter->fillRect((2), (height) - 3, 26, 1, colorHilite);
    painter->fillRect((1), (height) - 2, 318, 1, colorBase);
    painter->fillRect(0, (height) - 1, 319, 1, colorShadow);
}

void UltimateSoundTrackerPatternView::paintBelow(QPainter* painter, int height, int currentRow)
{
    int left = 3;

    QColor colorBase(156, 117, 82);
    QColor colorHilite(173, 138, 99);
    QColor colorShadow(82, 48, 16);
    QColor colorGreenHiliteColor(0, 173, 0);
    QColor colorGreenBaseColor(0, 140, 0);
    QColor colorGreenShadowColor(0, 99, 0);
    QPen pen(colorBase);
    pen.setWidth(1);
    painter->setPen(pen);
    int topOffset = -4;


    //left border
    pen.setColor(colorHilite);
    painter->setPen(pen);
    painter->drawLine(((left - 2)), 0, ((left - 2)), height);
    pen.setColor(colorBase);
    painter->setPen(pen);
    painter->drawLine(((left - 1)), 0, ((left - 1)), height);
    pen.setColor(colorShadow);
    painter->setPen(pen);
    painter->drawLine(((left - 0)), 0, ((left - 0)), height);


    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        //channel dividers
        pen.setColor(colorHilite);
        painter->setPen(pen);
        painter->drawLine((left + 25 + chan * 72), 0, (left + 25 + chan * 72), height);
        pen.setColor(colorBase);
        painter->setPen(pen);
        painter->drawLine((left + 26 + chan * 72), 0, (left + 26 + chan * 72), height);
        pen.setColor(colorShadow);
        painter->setPen(pen);
        painter->drawLine((left + 27 + chan * 72), 0, (left + 27 + chan * 72), height);

        //current row per channel

        int extra = 0;
        if (chan == m_channels - 1)
        {
            extra = 2;
        }
        //top hilite
        painter->fillRect(((left + 26)) + chan * 72, (height / 2) - 2 + (topOffset), (70 + extra), 2, colorHilite);
        //bottom shadow
        painter->fillRect(((left + 26)) + chan * 72, (height / 2) + 10 + (topOffset), (70 + extra), 2, colorShadow);
    }

    //right border
    pen.setColor(colorHilite);
    painter->setPen(pen);
    painter->drawLine(318, 0, 318, height);
    pen.setColor(colorBase);
    painter->setPen(pen);
    painter->drawLine(319, 0, 319, height);
    pen.setColor(colorShadow);
    painter->setPen(pen);
    painter->drawLine(320, 0, 320, height);


    topOffset = -2;
    painter->fillRect(((left - 2)), (height / 2) - 2 + (topOffset), 317, 10, colorBase);
    //vumeters base
    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        painter->fillRect(((left + 31)) + chan * 72 + 22, (height / 2) - 4 + (topOffset), 2, 1, colorGreenHiliteColor);
        painter->fillRect(((left + 33)) + chan * 72 + 22, (height / 2) - 4 + (topOffset), 4, 1, colorGreenBaseColor);
        painter->fillRect(((left + 37)) + chan * 72 + 22, (height / 2) - 4 + (topOffset), 2, 1, colorGreenShadowColor);
    }
    //leftmost current row

    topOffset = -4;
    painter->fillRect(((left - 1)), (height / 2) - 2 + (topOffset), 25, 2, colorHilite);
    painter->fillRect(((left - 1)), (height / 2) + 10 + (topOffset), 25, 2, colorShadow);
}

void::UltimateSoundTrackerPatternView::paintTop(QPainter* painter,Info* info, unsigned int m_currentPattern, unsigned int m_currentPosition, unsigned int m_currentSpeed, unsigned int m_currentBPM, unsigned int m_currentRow)
{
    m_height = 32;
    QColor colorBase(156, 117, 82);
    QColor colorHilite(173, 138, 99);
    QColor colorShadow(82, 48, 16);
    int top = 8;
    int left = 0;
    QRect rectBg(left, 0, 320, m_height);
    painter->fillRect(rectBg, colorBase);


    QRectF sourcePosition(0, 0, 47, 5);
    QRectF targetPosition(left + 5, 3, 47, 5);
    QRectF sourcePattern(0, 5, 48, 5);
    QRectF targetPattern(left + 111, 3, 48, 5);
    QRectF sourceLength(0, 10, 40, 5);
    QRectF targetLength(left + 218, 3, 40, 5);
    QRectF sourceSongname(0, 15, 61, 5);
    QRectF targetSongname(left + 50, 14, 61, 5);
    QRectF sourceSamplename(0, 20, 75, 5);
    QRectF targetSamplename(left + 50, 25, 75, 5);
    QRectF sourceNextSample(61, 0, 9, 7);
    QRectF targetNextSample(left + 2, 24, 9, 7);
    QRectF sourcePrevSample(70, 0, 9, 7);
    QRectF targetPrevSample(left + 14, 24, 9, 7);
    QString imagepath = dataPath + RESOURCES_DIR +
                        QDir::separator() + "trackerview" + QDir::separator() + "ust_top.png";
    QImage imageTop(imagepath);
    painter->drawImage(targetPosition, imageTop, sourcePosition);
    painter->drawImage(targetPattern, imageTop, sourcePattern);
    painter->drawImage(targetLength, imageTop, sourceLength);
    painter->drawImage(targetSongname, imageTop, sourceSongname);
    painter->drawImage(targetSamplename, imageTop, sourceSamplename);
    painter->drawImage(targetPrevSample, imageTop, sourcePrevSample);
    painter->drawImage(targetNextSample, imageTop, sourceNextSample);


    painter->setPen(QColor(0, 0, 0));
    drawText(QString("%1").arg(m_currentPosition, 4, 10, QChar('0')), painter, left + (71), top + 0, infoFont());
    drawText(QString("%1").arg(m_currentPattern, 4, 10, QChar('0')), painter, left + (178), top + 0, infoFont());
    drawText(QString("%1").arg(info->numOrders, 4, 10, QChar('0')), painter, left + (285), top + 0, infoFont());
    drawText(QString("%1").arg(info->title.c_str(), -20, QChar('_')).toUpper(), painter, left + (143),
             top + (11), infoFont());
    drawText(QString("%1").arg(getCurrentSample() + 1, 2, 16, QChar('0')).toUpper(), painter,
             left + (26), top + (22), infoFont());
    drawText(
            QString("%1").arg(info->samples[getCurrentSample()].c_str(), -22, QChar('_')).
                    toUpper(), painter, left + (143), top + (22), infoFont());
    m_pen.setWidth(1);

    m_pen.setColor(colorHilite);
    painter->setPen(m_pen);
    painter->drawLine(left, 0, left + 319, 0);

    m_pen.setColor(colorShadow);
    painter->setPen(m_pen);
    painter->drawLine(left, 10, left + (319), 10);
    painter->drawLine(left, 21, left + (319), 21);

    m_pen.setColor(colorHilite);
    painter->setPen(m_pen);
    painter->drawLine(left, 11, left + 319, 11);
    painter->drawLine(left, 22, left + 319, 22);

    drawVerticalEmboss(left + 106, 1, 9, colorHilite, colorShadow, colorShadow, painter);

    drawVerticalEmboss(left + 213, 1, 9, colorHilite, colorShadow, colorShadow, painter);
    drawVerticalEmboss(left + 45, 22, 10, colorHilite, colorShadow, colorShadow, painter);


    //far left emboss
    m_pen.setColor(colorHilite);
    painter->setPen(m_pen);
    painter->drawLine(left + 1, 1, left + 1, 9);
    m_pen.setColor(colorHilite);
    painter->setPen(m_pen);
    painter->drawLine(left + 1, 12, left + 1, 32);

    //far right emboss
    m_pen.setColor(colorShadow);
    painter->setPen(m_pen);
    painter->drawLine(left + 320, 0, left + (320), 32);
}
UltimateSoundTrackerPatternView::~UltimateSoundTrackerPatternView()
{
}
