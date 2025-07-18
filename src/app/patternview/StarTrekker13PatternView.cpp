#include "StarTrekker13PatternView.h"
#include <QDir>
#include "mainwindow.h"
StarTrekker13PatternView::StarTrekker13PatternView(Tracker* parent, unsigned int channels)
    : AbstractPatternView(parent, channels)
{
    rowNumberOffset = 0;
    octaveOffset = 48;
    m_font2 = QFont("Startrekker 1.3 Double Height");
    m_font2.setPixelSize(14);
    m_font = QFont("Startrekker 1.3");
    m_font.setPixelSize(7);
    m_fontWidth = 8;

    m_bitmapFont = BitmapFont("Startrekker 1.3");
    m_bitmapFont2 = BitmapFont("Startrekker 1.3 Double Height");
    m_fontWidth = 8;
    m_fontHeight = 7;

    m_font.setStyleStrategy(QFont::NoAntialias);
    m_colorCurrentRowBackground = QColor(189, 138, 99);
    m_colorDefault = m_ColorRowNumber = m_ColorInstrument = m_ColorEffect = m_ColorParameter = m_ColorEffect2 =
    m_ColorParameter2 = m_ColorVolume = m_colorEmpty = QColor(156, 170, 33);

    m_fontWidth = 8;
    m_font.setStyleStrategy(QFont::NoAntialias);
    m_RowEnd = m_SeparatorRowNumber = m_SeparatorChannel = "'";
    m_RowLength = 40;
    m_xOffsetRow = 8;
    m_xChannelStart = 29;
    m_channelWidth = 70;
    m_channelLastWidth = 72;
    m_channelxSpace = 2;

    m_ibuttonPrevSampleWidth = 11;
    m_ibuttonPrevSampleHeight = 11;
    m_ibuttonPrevSampleX = 11;
    m_ibuttonPrevSampleY = 33;
    m_ibuttonNextSampleWidth = 11;
    m_ibuttonNextSampleHeight = 11;
    m_ibuttonNextSampleX = 0;
    m_ibuttonNextSampleY = 33;
    m_topHeight = 44;

    m_vumeterTopOffset = -5;

    //main color
    m_linearGrad.setColorAt(0, QColor(0, 0, 206).rgb());
    m_linearGrad.setColorAt(1, QColor(33, 222, 222).rgb());

    //hilight color (left)
    m_linearGradHiLite.setColorAt(0, QColor(0, 0, 239).rgb());
    m_linearGradHiLite.setColorAt(1, QColor(49, 255, 255).rgb());

    //dark color (right)
    m_linearGradDark.setColorAt(0, QColor(0, 0, 156).rgb());
    m_linearGradDark.setColorAt(1, QColor(16, 189, 189).rgb());
}

QFont StarTrekker13PatternView::currentRowFont()
{
    return m_font2;
}

BitmapFont StarTrekker13PatternView::currentRowBitmapFont()
{
    return m_bitmapFont2;
}

StarTrekker13PatternView::~StarTrekker13PatternView()
{
}

void StarTrekker13PatternView::paintAbove(QPainter* painter, int height, int currentRow)
{
    QColor colorBase(189, 138, 99);
    QColor colorHilite(222, 186, 156);
    QColor colorShadow(140, 85, 49);

    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        //current row per channel

        int extra = 0;
        if (chan == m_channels - 1)
        {
            extra = 2;
        }
        //bottom hilight
        painter->fillRect((29) + chan * 72, (height) - 3, 1, 1, colorShadow);
        painter->fillRect((30) + chan * 72, (height) - 3, (70 + extra), 1, colorHilite);
    }

    painter->fillRect((3), (height) - 3, 25, 1, colorHilite);
    painter->fillRect((1), (height) - 2, 318, 1, colorBase);
    painter->fillRect(1, (height) - 1, 319, 1, colorShadow);
}

void StarTrekker13PatternView::paintBelow(QPainter* painter, int height, int currentRow)
{
    int left = 3;

    QColor colorBase(189, 138, 99);
    QColor colorHilite(222, 186, 156);
    QColor colorShadow(140, 85, 49);

    QPen pen(colorBase);
    pen.setWidth(1);
    painter->setPen(pen);
    int topOffset = -4;


    //left border
    pen.setColor(colorHilite);
    painter->setPen(pen);
    painter->drawLine(((left - 2)), 45, ((left - 2)), height);
    pen.setColor(colorBase);
    painter->setPen(pen);
    painter->drawLine(((left - 1)), 45, ((left - 1)), height);
    pen.setColor(colorShadow);
    painter->setPen(pen);
    painter->drawLine(((left)), 45, ((left)), height);


    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        //channel dividers
        pen.setColor(colorHilite);
        painter->setPen(pen);
        painter->drawLine((left + 25 + chan * 72), 45, (left + 25 + chan * 72), height);
        pen.setColor(colorBase);
        painter->setPen(pen);
        painter->drawLine((left + 26 + chan * 72), 45, (left + 26 + chan * 72), height);
        pen.setColor(colorShadow);
        painter->setPen(pen);
        painter->drawLine((left + 27 + chan * 72), 45, (left + 27 + chan * 72), height);

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
    painter->drawLine(318, 45, 318, height);
    pen.setColor(colorBase);
    painter->setPen(pen);
    painter->drawLine(319, 45, 319, height);
    pen.setColor(colorShadow);
    painter->setPen(pen);
    painter->drawLine(320, 45, 320, height);


    topOffset = -2;
    painter->fillRect(((left - 2)), (height / 2) - 2 + (topOffset), 317, 10, colorBase);

    //bottom shadow
    topOffset = -4;
    painter->fillRect((2), (height / 2) + 10 + (topOffset), 26, 2, colorShadow);


    //leftmost current row

    topOffset = -4;
    painter->fillRect(((left)), (height / 2) - 2 + (topOffset), 24, 2, colorHilite);
    //leftmost current ror antialias
    painter->fillRect(((left - 1)), (height / 2) - 2 + (topOffset), 1, 2, colorBase);
}
void::StarTrekker13PatternView::paintTop(QPainter* painter,Info* info, unsigned int m_currentPattern, unsigned int m_currentPosition, unsigned int m_currentSpeed, unsigned int m_currentBPM, unsigned int m_currentRow)
{
    m_height = 44;
    QColor colorBase(189, 138, 99);
    QColor colorHilite(222, 186, 156);
    QColor colorShadow(140, 85, 49);
    int top = 8;
    int left = 0;
    QRect rectBg(left, 0, 320, m_height);
    painter->fillRect(rectBg, colorBase);

    painter->setPen(colorShadow);
    drawText("POSITION", painter, left + (4), top + 1, infoFont(), -1);
    painter->setPen(QColor(0, 0, 0));

    drawText(QString("%1").arg(m_currentPosition, 4, 10, QChar('0')), painter, left + 71, top + 0, infoFont());
    painter->setPen(colorHilite);
    drawText("POSITION", painter, left + (3), top + 0, infoFont(),-1);
    painter->setPen(colorShadow);
    drawText("PATTERN", painter, left + (111), top + 1, infoFont(),-1);
    painter->setPen(colorHilite);
    drawText("PATTERN", painter, left + (110), top + 0, infoFont(),-1);
    painter->setPen(QColor(0, 0, 0));

    drawText(QString("%1").arg(m_currentPattern, 4, 10, QChar('0')), painter, left + (178), top + 0, infoFont());
    painter->setPen(colorShadow);
    drawText("LENGTH", painter, left + (4), top + (11) + 1, infoFont(),-1);
    painter->setPen(colorHilite);
    drawText("LENGTH", painter, left + (3), top + (11), infoFont(),-1);
    painter->setPen(QColor(0, 0, 0));
    Tracker* t = (Tracker*)this->parent();
    drawText(QString("%1").arg(t->m_info->numOrders, 4, 10, QChar('0')), painter, left + (71), top + (11), infoFont());
    painter->setPen(colorShadow);
    drawText("RESTART", painter, left + (111), top + (11) + 1, infoFont(),-1);
    painter->setPen(colorHilite);
    drawText("RESTART", painter, left + (110), top + (11), infoFont(),-1);
    painter->setPen(QColor(0, 0, 0));
    drawText(QString("%1").arg(t->m_info->restart, 4, 10, QChar('0')), painter, left + (178), top + (11), infoFont());

    painter->setPen(colorShadow);
    drawText("SONGNAME:", painter, left + 4, top + 23, infoFont(),-1);
    painter->setPen(colorHilite);
    drawText("SONGNAME:", painter, left + (3), top + (22), infoFont(),-1);
    painter->setPen(QColor(0, 0, 0));

    drawText(QString("%1").arg(t->m_info->title.c_str(), -20, QChar('_')).toUpper(), painter, left + (72),
             top + (22), infoFont());

    m_pen.setWidth(1);

    m_pen.setColor(colorHilite);
    painter->setPen(m_pen);
    painter->drawLine(left + 1, 0, left + (319), 0);

    m_pen.setColor(colorShadow);
    painter->setPen(m_pen);
    painter->drawLine(left + 1, 10, left + (215), 10);
    painter->drawLine(left + 1, 21, left + (319), 21);
    painter->drawLine(left + 1, 32, left + (319), 32);

    m_pen.setColor(colorHilite);
    painter->setPen(m_pen);
    painter->drawLine(left + 1, (11), left + (215), (11));
    painter->drawLine(left + 1, (22), left + (319), (22));
    painter->drawLine(left + 1, (33), left + (319), (33));

    drawVerticalEmboss(left + 68, 0, 10, colorHilite, colorShadow, colorBase, painter);
    drawVerticalEmboss(left + 107, 0, 10, colorHilite, colorShadow, colorBase, painter);
    drawVerticalEmboss(left + 175, 0, 10, colorHilite, colorShadow, colorBase, painter);
    drawVerticalEmboss(left + 214, 0, 10, colorHilite, colorShadow, colorBase, painter, true, false);
    drawVerticalEmboss(left + 68, 11, 10, colorHilite, colorShadow, colorBase, painter);
    drawVerticalEmboss(left + 107, 11, 10, colorHilite, colorShadow, colorBase, painter);
    drawVerticalEmboss(left + 175, 11, 10, colorHilite, colorShadow, colorBase, painter);
    drawVerticalEmboss(left + 214, 11, 10, colorHilite, colorShadow, colorBase, painter, true, false);
    drawVerticalEmboss(left + 214, 0, 21, colorHilite, colorShadow, colorBase, painter, false, true);
    drawVerticalEmboss(left + 10, 33, 10, colorHilite, colorShadow, colorBase, painter, true, true);
    drawVerticalEmboss(left + 21, 33, 10, colorHilite, colorShadow, colorBase, painter, true, true);
    drawVerticalEmboss(left + 44, 33, 10, colorHilite, colorShadow, colorBase, painter, true, true);

    //far left emboss
    drawVerticalEmboss(left - 1, 0, 10, colorHilite, colorShadow, colorBase, painter, false, true);
    drawVerticalEmboss(left - 1, 11, 10, colorHilite, colorShadow, colorBase, painter, false, true);
    drawVerticalEmboss(left - 1, 22, 10, colorHilite, colorShadow, colorBase, painter, false, true);
    drawVerticalEmboss(left - 1, 33, 10, colorHilite, colorShadow, colorBase, painter, false, true);

    //far right emboss
    drawVerticalEmboss(left + (319), 0, 22, colorHilite, colorShadow, colorBase, painter, true, false);
    drawVerticalEmboss(left + (319), 22, 11, colorHilite, colorShadow, colorBase, painter, true, false);
    drawVerticalEmboss(left + (319), 33, 11, colorHilite, colorShadow, colorBase, painter, true, false);


    QRectF sourcePrev(0, 0, 6, 7);
    QRectF sourceNext(6, 0, 6, 7);
    QRectF targetPrev(left + 3, 35, 6, 7);
    QRectF targetNext(left + 14, 35, 6, 7);
    QString imagepath = dataPath + RESOURCES_DIR +
                        QDir::separator() + "trackerview" + QDir::separator() + "startrekker_top.png";
    QImage imageTop(imagepath);
    painter->drawImage(targetPrev, imageTop, sourcePrev);
    painter->drawImage(targetNext, imageTop, sourceNext);

    painter->setPen(colorShadow);
    drawText("SAMPLENAME:", painter, left + (59), top + (33) + 1, infoFont(),-1);
    painter->setPen(colorHilite);
    drawText("SAMPLENAME:", painter, left + (58), top + (33), infoFont(),-1);
    painter->setPen(QColor(0, 0, 0));

    drawText(QString("%1").arg(getCurrentSample() + 1, 2, 10, QChar('0')), painter,
             left + (24), top + (33), infoFont());
    drawText(
            QString("%1").arg(t->m_info->samples[getCurrentSample()].c_str(), -22, QChar('_')).
                    toUpper(), painter, left + (141), top + (33), infoFont());

    //1px antialias
    painter->fillRect(left, 32, 1, 1, colorBase);
    painter->fillRect(left + 319, 33, 1, 1, colorBase);

    //bottom shadow
    painter->fillRect(left, 43, 320, 1, colorShadow);
}