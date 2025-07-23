#include "ChipTrackerPatternView.h"
#include <QDir>
#include "mainwindow.h"

ChipTrackerPatternView::ChipTrackerPatternView(Tracker* parent, unsigned int channels)
    : AbstractPatternView(parent, channels)
{
    rowNumberOffset = 0;
    octaveOffset = 12;
    m_font2 = QFont("Chiptracker Double Height");
    m_font2.setPixelSize(14);
    m_colorDefault = m_ColorRowNumber = m_ColorInstrument = m_ColorEffect = m_ColorParameter = m_ColorEffect2 =
        m_ColorParameter2 = m_ColorVolume = m_colorEmpty = QColor(33, 32, 255);
    m_font = QFont("Chiptracker");
    m_font.setPixelSize(7);
    m_fontWidth = 8;
    m_font.setStyleStrategy(QFont::NoAntialias);

    m_bitmapFont = BitmapFont("Chiptracker");
    m_fontWidth = 8;
    m_fontHeight = 7;
    m_bitmapFont2 = BitmapFont("Chiptracker Double Height");

    m_RowEnd = m_SeparatorRowNumber = m_SeparatorChannel = " ";
    m_RowLength = 40;
    m_xOffsetRow = 8;
    m_yOffsetCurrentRowBefore = -1;
    m_xChannelStart = 29;
    m_channelWidth = 70;
    m_channelLastWidth = 72;
    m_channelxSpace = 2;

    m_ibuttonPrevSampleWidth = 11;
    m_ibuttonPrevSampleHeight = 11;
    m_ibuttonPrevSampleX = 11;
    m_ibuttonPrevSampleY = 22;
    m_ibuttonNextSampleWidth = 11;
    m_ibuttonNextSampleHeight = 11;
    m_ibuttonNextSampleX = 0;
    m_ibuttonNextSampleY = 22;


    //main color
    m_linearGrad.setColorAt(0, QColor(189, 16, 0).rgb()); //red
    m_linearGrad.setColorAt(0.11, QColor(189, 50, 0).rgb()); //red
    m_linearGrad.setColorAt(0.36, QColor(189, 255, 0).rgb()); //yellow
    m_linearGrad.setColorAt(0.81, QColor(0, 255, 0).rgb()); // green
    m_linearGrad.setColorAt(0.87, QColor(0, 227, 0).rgb()); // green
    m_linearGrad.setColorAt(0.95, QColor(0, 169, 0).rgb()); // green


    //hilight color (left)
    m_linearGradHiLite.setColorAt(0, QColor(255, 16, 0).rgb()); //red
    m_linearGradHiLite.setColorAt(0.11, QColor(255, 50, 0).rgb()); //red
    m_linearGradHiLite.setColorAt(0.36, QColor(255, 255, 0).rgb()); //yellow
    m_linearGradHiLite.setColorAt(0.81, QColor(66, 255, 0).rgb()); // green
    m_linearGradHiLite.setColorAt(1, QColor(0, 239, 0).rgb()); // dark green

    //dark color (right)
    m_linearGradDark.setColorAt(0, QColor(115, 16, 0).rgb()); //red
    m_linearGradDark.setColorAt(0.11, QColor(111, 50, 0).rgb()); //red
    m_linearGradDark.setColorAt(0.36, QColor(115, 255, 0).rgb()); //yellow
    m_linearGradDark.setColorAt(0.74, QColor(0, 218, 0).rgb()); // green
    m_linearGradDark.setColorAt(0.85, QColor(0, 166, 0).rgb()); // green
    m_linearGradDark.setColorAt(0.93, QColor(0, 105, 0).rgb()); // green
    m_linearGradDark.setColorAt(1, QColor(0, 96, 0).rgb()); // dark green
}

QFont ChipTrackerPatternView::currentRowFont()
{
    return m_font2;
}

BitmapFont ChipTrackerPatternView::currentRowBitmapFont()
{
    return m_bitmapFont2;
}

ChipTrackerPatternView::~ChipTrackerPatternView()
{
}

void ChipTrackerPatternView::paintAbove(QPainter* painter, int height, int currentRow)
{
    QColor colorBase(115, 117, 115);
    QColor colorHilite(170, 173, 170);
    QColor colorShadow(66, 69, 66);

    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        int extra = 0;
        if (chan == m_channels - 1)
        {
            extra = 2;
        }
        painter->fillRect((30) + chan * 72, (height) - 3, (70 + extra), 1, colorHilite);
        painter->fillRect((29) + chan * 72, (height) - 3, 1, 1, colorBase);
        painter->fillRect((29) + chan * 72, 32, (70 + extra), 1, colorShadow);
    }
    painter->fillRect((2), 32, 25, 1, colorShadow);
    painter->fillRect((3), (height) - 3, 25, 1, colorHilite);
    painter->fillRect((1), (height) - 2, 318, 1, colorBase);
    painter->fillRect(1, (height) - 1, 319, 1, colorShadow);
    painter->fillRect(2, (height) - 3, 1, 1, colorBase);

    //1px antialias
    painter->fillRect(11, 32, 1, 1, colorBase);
    painter->fillRect(22, 32, 1, 1, colorBase);
    painter->fillRect(59, 32, 1, 1, colorBase);
}

void ChipTrackerPatternView::paintBelow(QPainter* painter, int height, int currentRow)
{
    int left = 3;

    QColor colorBase(115, 117, 115);
    QColor colorHilite(170, 173, 170);
    QColor colorShadow(66, 69, 66);
    QPen pen(colorBase);
    pen.setWidth(1);
    painter->setPen(pen);
    int topOffset = -4;


    //left border
    pen.setColor(colorHilite);
    painter->setPen(pen);
    painter->drawLine(((left - 2)), 0, (left - 3), height);
    pen.setColor(colorBase);
    painter->setPen(pen);
    painter->drawLine(((left - 1)), 0, (left - 2), height);
    pen.setColor(colorShadow);
    painter->setPen(pen);
    painter->drawLine(left - 0, 0, left - 0, height - 4);

    //1 pixel bottom antialiasing
    painter->fillRect((left - 1), height - 3, 1, 1, colorBase);
    //1 pixel bottom antialiasing
    painter->fillRect((left - 3), height - 1, 1, 1, colorBase);


    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        QColor colorRed(255, 0, 0);
        //channel dividers
        pen.setColor(colorHilite);
        painter->setPen(pen);
        painter->drawLine((left + 25 + chan * 72), 32, (left + 25 + chan * 72), height);
        pen.setColor(colorBase);
        painter->setPen(pen);
        painter->drawLine((left + 26 + chan * 72), 32, (left + 26 + chan * 72), height);
        pen.setColor(colorShadow);
        painter->setPen(pen);
        painter->drawLine((left + 27 + chan * 72), 32, (left + 27 + chan * 72), (height) - 4);
        //1 pixel top antialiasing
        painter->fillRect((left + 24 + chan * 72), 32, 1, 1, colorBase);
        //1 pixel bottom antialiasing
        painter->fillRect((left + 26 + chan * 72), (height) - 2, 1, 1, colorBase);
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
    //1 pixel antialiasing
    painter->fillRect(317, 0, 1, 1, colorBase);
    painter->fillRect(317, 32, 1, 1, colorBase);

    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        //current row per channel

        int extra = 0;
        if (chan == m_channels - 1)
        {
            extra = 1;
        }

        //top hilite
        painter->fillRect(((left + 26)) + chan * 72, (height / 2) - 2 + (topOffset), (71 + extra), 2, colorHilite);
        //bottom shadow
        painter->fillRect(((left + 26)) + chan * 72, (height / 2) + 10 + (topOffset), (71 + extra), 2, colorShadow);
    }


    //current row left
    //top hilite
    painter->fillRect(((left - 1)), (height / 2) - 2 + (topOffset), 26, 2, colorHilite);
    //bottom shadow
    painter->fillRect(((left - 1)), (height / 2) + 10 + (topOffset), 26, 2, colorShadow);

    //main current row
    painter->fillRect(((left - 1)), (height / 2) - 4, 317, 10, colorBase);

    //right antialias
    painter->fillRect(317, (height / 2) - 6, 1, 14, colorBase);

    QColor colorGreenHiliteColor(0, 239, 0);
    QColor colorGreenBaseColor(0, 170, 0);
    QColor colorGreenShadowColor(0, 101, 0);
    //vumeters base
    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        painter->fillRect(((left + 30)) + chan * 72 + 22, (height / 2) - 2 + (topOffset), 2, 1, colorGreenHiliteColor);
        painter->fillRect(((left + 32)) + chan * 72 + 22, (height / 2) - 2 + (topOffset), 6, 1, colorGreenBaseColor);
        painter->fillRect(((left + 38)) + chan * 72 + 22, (height / 2) - 2 + (topOffset), 2, 1, colorGreenShadowColor);
    }
}
void::ChipTrackerPatternView::paintTop(QPainter* painter,Info* info, unsigned int m_currentPattern, unsigned int m_currentPosition, unsigned int m_currentSpeed, unsigned int m_currentBPM, unsigned int m_currentRow)
{
    m_topHeight = 32;
    QColor colorBase(115, 117, 115);
    QColor colorHilite(173, 170, 173);
    QColor colorShadow(66, 69, 66);
    int top = 8;
    int left = 0;
    QRect rectBg(left, 0, 320, m_topHeight);
    painter->fillRect(rectBg, colorBase);
    QRectF sourcePosition(0, 0, 60, 6);
    QRectF targetPosition(left + 4, 3, 60, 6);
    QRectF sourcePattern(0, 6, 55, 6);
    QRectF targetPattern(left + 111, 3, 55, 6);
    QRectF sourceLength(0, 12, 48, 6);
    QRectF targetLength(left + 218, 3, 48, 6);
    QRectF sourceSongname(0, 18, 70, 6);
    QRectF targetSongname(left + 62, 14, 70, 6);
    QRectF sourcePrev(58, 6, 6, 7);
    QRectF sourceNext(64, 6, 6, 7);
    QRectF targetPrev(left + 3, 24, 6, 7);
    QRectF targetNext(left + 14, 24, 6, 7);

    QString imagepath = dataPath + RESOURCES_DIR +
                        QDir::separator() + "trackerview" + QDir::separator() + "chiptracker_top.png";
    QImage imageTop(imagepath);
    painter->drawImage(targetPosition, imageTop, sourcePosition);
    painter->drawImage(targetPattern, imageTop, sourcePattern);
    painter->drawImage(targetLength, imageTop, sourceLength);
    painter->drawImage(targetSongname, imageTop, sourceSongname);
    painter->drawImage(targetPrev, imageTop, sourcePrev);
    painter->drawImage(targetNext, imageTop, sourceNext);

    painter->setPen(QColor(0, 0, 0));

    Tracker* t = (Tracker*)this->parent();
    drawText(QString("%1").arg(m_currentPosition, 4, 16, QChar('0')).toUpper(), painter, left + 71,
             top + 0, infoFont());
    drawText(QString("%1").arg(t->m_info->restart, 4, 16, QChar('0')).toUpper(), painter, left + (178),
             top + 0, infoFont());
    drawText(QString("%1").arg(t->m_info->numOrders, 4, 16, QChar('0')).toUpper(), painter, left + (285),
             top + 0, infoFont());
    drawText(QString("%1").arg(t->m_info->title.c_str(), -22, QChar('_')).toUpper(), painter, left + (137),
             top + (11), infoFont());

    m_pen.setWidth(1);

    m_pen.setColor(colorHilite);
    painter->setPen(m_pen);
    painter->drawLine(left + 1, 0, 319, 0);

    m_pen.setColor(colorShadow);
    painter->setPen(m_pen);
    painter->drawLine(left + 1, 10, 319, 10);

    m_pen.setColor(colorHilite);
    painter->setPen(m_pen);
    painter->drawLine(left + 1, 11, 319, 11);

    m_pen.setColor(colorShadow);
    painter->setPen(m_pen);
    painter->drawLine(left + 1, 21, 319, 21);

    m_pen.setColor(colorHilite);
    painter->setPen(m_pen);
    painter->drawLine(left + 1, 22, 319, 22);

    drawVerticalEmboss(left + 68, 0, 10, colorHilite, colorShadow, colorBase, painter);
    drawVerticalEmboss(left + 107, 0, 10, colorHilite, colorShadow, colorBase, painter);
    drawVerticalEmboss(left + 175, 0, 10, colorHilite, colorShadow, colorBase, painter);
    drawVerticalEmboss(left + 214, 0, 10, colorHilite, colorShadow, colorBase, painter);
    drawVerticalEmboss(left + 281, 0, 10, colorHilite, colorShadow, colorBase, painter);
    drawVerticalEmboss(left + 10, 22, 10, colorHilite, colorShadow, colorBase, painter);
    drawVerticalEmboss(left + 21, 22, 10, colorHilite, colorShadow, colorBase, painter);
    drawVerticalEmboss(left + 58, 22, 10, colorHilite, colorShadow, colorBase, painter);

    //far left emboss
    drawVerticalEmboss(left - 1, 0, 10, colorHilite, colorShadow, colorBase, painter, false, true);

    m_pen.setColor(colorHilite);
    painter->setPen(m_pen);
    painter->drawLine(left + 1, 11, left + 1, 33);

    //far right emboss
    drawVerticalEmboss(left + (319), 0, 11, colorHilite, colorShadow, colorBase, painter, true, false);
    drawVerticalEmboss(left + (319), 11, 21, colorHilite, colorShadow, colorBase, painter, true, false);

    //1px antialias
    painter->fillRect(left + (319), 22, 1, 1, colorBase);
    painter->fillRect(left, 21, 1, 1, colorBase);

    painter->setPen(colorShadow);
    drawText("SAMPNAME:", painter, left + (63), top + 23, infoFont());
    painter->setPen(colorHilite);
    drawText("SAMPNAME:", painter, left + (62), top + 22, infoFont());
    painter->setPen(QColor(0, 0, 0));
    drawText(QString("%1").arg(getCurrentSample() + 1, 4, 16, QChar('0')).toUpper(), painter,
             left + (24), top + (22), infoFont());
    drawText(
            QString("%1").arg(t->m_info->samples[getCurrentSample()].c_str(), -22, QChar('_')).
                    toUpper(), painter, left + (137), top + (22), infoFont());
}