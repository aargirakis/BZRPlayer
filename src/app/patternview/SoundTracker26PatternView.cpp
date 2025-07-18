#include "SoundTracker26PatternView.h"
#include <QDir>
#include "mainwindow.h"

SoundTracker26PatternView::SoundTracker26PatternView(Tracker* parent, unsigned int channels)
    : NoiseTrackerPatternView(parent, channels)
{
    m_ibuttonPrevSampleWidth = 11;
    m_ibuttonPrevSampleHeight = 11;
    m_ibuttonPrevSampleX = 11;
    m_ibuttonPrevSampleY = 22;
    m_ibuttonNextSampleWidth = 11;
    m_ibuttonNextSampleHeight = 11;
    m_ibuttonNextSampleX = 0;
    m_ibuttonNextSampleY = 22;

    m_vumeterHeight = 48;
    m_vumeterWidth = 22;
    m_vumeterLeftOffset = 61;


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

SoundTracker26PatternView::~SoundTracker26PatternView()
{
}

void SoundTracker26PatternView::paintBelow(QPainter* painter, int height, int currentRow)
{
    QColor colorHilite(173, 170, 173);
    NoiseTrackerPatternView::paintBelow(painter, height, currentRow);
    painter->fillRect(2, (height / 2) - 6, 1, 2, colorHilite);
}
void::SoundTracker26PatternView::paintTop(QPainter* painter,Info* info, unsigned int m_currentPattern, unsigned int m_currentPosition, unsigned int m_currentSpeed, unsigned int m_currentBPM, unsigned int m_currentRow)
{
    m_height = 32;
    QColor colorBase(115, 117, 115);
    QColor colorHilite(173, 170, 173);
    QColor colorShadow(66, 69, 66);
    int top = 8;
    int left = 0;
    QRect rectBg(left, 0, 320, m_height);
    painter->fillRect(rectBg, colorBase);

    painter->setPen(colorShadow);
    drawText("POSITION", painter, left + (4), top + 1, infoFont(), -1);
    painter->setPen(QColor(0, 0, 0));

    drawText(QString("%1").arg(m_currentPosition, 4, 10, QChar('0')), painter, left + (71), top + 0, infoFont());
    painter->setPen(colorHilite);
    drawText("POSITION", painter, left + (3), top + 0, infoFont(),-1);
    painter->setPen(colorShadow);
    drawText("LENGTH", painter, left + (111), top + 1, infoFont(),-1);
    painter->setPen(colorHilite);
    drawText("LENGTH", painter, left + (110), top + 0, infoFont(),-1);
    painter->setPen(QColor(0, 0, 0));

    Tracker* t = (Tracker*)this->parent();
    drawText(QString("%1").arg(t->m_info->numOrders, 4, 10, QChar('0')), painter, left + (178), top + 0, infoFont());
    painter->setPen(colorShadow);
    drawText("SONGNAME:", painter, left + (73), top + (11) + 1, infoFont(),-1);
    painter->setPen(colorHilite);
    drawText("SONGNAME:", painter, left + (72), top + (11), infoFont(),-1);
    painter->setPen(QColor(0, 0, 0));

    drawText(QString("%1").arg(t->m_info->title.c_str(), -20, QChar('_')).toUpper(), painter, left + (141),
             top + (11), infoFont());
    m_pen.setWidth(1);

    m_pen.setColor(colorHilite);
    painter->setPen(m_pen);
    painter->drawLine(left + 1, 0, left + (319), 0);

    m_pen.setColor(colorShadow);
    painter->setPen(m_pen);
    painter->drawLine(left + 1, 10, left + (319), 10);

    m_pen.setColor(colorHilite);
    painter->setPen(m_pen);
    painter->drawLine(left + 1, 11, left + (319), 11);

    m_pen.setColor(colorShadow);
    painter->setPen(m_pen);
    painter->drawLine(left + 1, 21, left + (319), 21);

    m_pen.setColor(colorHilite);
    painter->setPen(m_pen);
    painter->drawLine(left, 22, left + (319), 22);

    drawVerticalEmboss(left + 10, 22, 10, colorHilite, colorShadow, colorBase, painter);
    drawVerticalEmboss(left + 21, 22, 10, colorHilite, colorShadow, colorBase, painter);
    drawVerticalEmboss(left + 44, 22, 10, colorHilite, colorShadow, colorBase, painter);
    drawVerticalEmboss(left + 68, 0, 10, colorHilite, colorShadow, colorBase, painter);
    drawVerticalEmboss(left + 107, 0, 10, colorHilite, colorShadow, colorBase, painter);
    drawVerticalEmboss(left + 175, 0, 10, colorHilite, colorShadow, colorBase, painter);
    drawVerticalEmboss(left + 214, 0, 10, colorHilite, colorShadow, colorBase, painter);

    //far left emboss
    drawVerticalEmboss(left - 1, 0, 10, colorHilite, colorShadow, colorBase, painter, false, true);
    drawVerticalEmboss(left - 1, 11, 10, colorHilite, colorShadow, colorBase, painter, false, true);
    drawVerticalEmboss(left - 1, 22, 10, colorHilite, colorShadow, colorBase, painter, false, true);

    //far right emboss
    drawVerticalEmboss(left + (319), 0, 11, colorHilite, colorShadow, colorBase, painter, true, false);
    drawVerticalEmboss(left + (319), 11, 11, colorHilite, colorShadow, colorBase, painter, true, false);
    drawVerticalEmboss(left + (319), 22, 11, colorHilite, colorShadow, colorBase, painter, true, false);

    painter->setPen(colorShadow);
    drawText("SAMPLENAME:", painter, left + (59), top + (22) + 1, infoFont(), -1);
    painter->setPen(colorHilite);
    drawText("SAMPLENAME:", painter, left + (58), top + (22), infoFont(), -1);
    painter->setPen(QColor(0, 0, 0));

    drawText(QString("%1").arg(getCurrentSample() + 1, 2, 10, QChar('0')), painter,
             left + (24), top + (22), infoFont());
    drawText(
            QString("%1").arg(t->m_info->samples[getCurrentSample()].c_str(), -22, QChar('_')).
                    toUpper(), painter, left + (141), top + (22), infoFont());
    QRectF sourcePrev(0, 0, 6, 7);
    QRectF sourceNext(6, 0, 6, 7);
    QRectF targetPrev(left + 3, 24, 6, 7);
    QRectF targetNext(left + 14, 24, 6, 7);
    QString imagepath = dataPath + RESOURCES_DIR +
                        QDir::separator() + "trackerview" + QDir::separator() + "noisetracker_top.png";
    QImage imageTop(imagepath);
    painter->drawImage(targetPrev, imageTop, sourcePrev);
    painter->drawImage(targetNext, imageTop, sourceNext);
}