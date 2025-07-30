#include "HivelyTrackerPatternView.h"
#include "mainwindow.h"
#include <QDir>
HivelyTrackerPatternView::HivelyTrackerPatternView(Tracker* parent, unsigned int channels)
    : AbstractPatternView(parent, channels)
{
    instrumentPad = true;
    instrumentHex = false;
    rowNumberOffset = 0;
    effect2Enabled = true;
    parameter2Enabled = true;
    m_emptyInstrument = "  ";
    m_font = QFont("DejaVu Sans Mono");
    m_font.setPixelSize(14);
    m_fontWidth = 8;
    m_fontHeight = 14;
    m_yOffsetRowAfter = 2;

    m_renderTop = true;

    m_colorCurrentRowBackground = QColor(68, 102, 136);
    m_colorCurrentRowForeground = m_colorDefault = m_ColorRowNumber = m_ColorInstrument = m_ColorEffect =
        m_ColorParameter = m_ColorEffect2 = m_ColorParameter2 = m_ColorVolume = m_colorEmpty = QColor(170, 204, 238);
    m_SeparatorInstrument = m_SeparatorParameter = m_SeparatorNote = " ";
    m_SeparatorChannel = m_RowEnd = m_SeparatorRowNumber = " ";
    m_RowLength = (4 + m_channels * 15);
    m_xOffsetRow = 3;
    m_width = 640;
    m_height = 480;

    //for dimming channels
    m_xChannelStart = 24;
    m_channelWidth = 119;
    m_channelLastWidth = m_channelWidth;
    m_channelxSpace = 1;
    m_topHeight = 104;
    m_bottomFrameHeight = 4;

    m_vumeterHeight = 64;
    m_vumeterWidth = 32;
    m_vumeterLeftOffset = 67;
    m_vumeterOffset = 120;
    m_vumeterHilightWidth = 1;
    m_vumeterTopOffset = -13;

    setupVUMeters();
}

void HivelyTrackerPatternView::paintBelow(QPainter* painter, int height, int currentRow)
{
    QColor colorBase(68, 102, 136);
    QColor colorHilite(136, 153, 187);
    QColor colorShadow(17, 51, 85);

    QPen pen(colorShadow);
    pen.setWidth(1);
    painter->setPen(pen);

    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        //channel dividers
        pen.setColor(QColor(68, 102, 136));
        painter->setPen(pen);
        painter->drawLine((56 + chan * 120), 0, (56 + chan * 120), height - 6);
        painter->drawLine((80 + chan * 120), 0, (80 + chan * 120), height - 6);
        painter->drawLine((112 + chan * 120), 0, (112 + chan * 120), height - 6);
    }

    //main
    painter->fillRect(3, (height / 2) - 13, (22 + m_channels * 120), 15, m_colorCurrentRowBackground);
    painter->fillRect(3, (height / 2) - 14, (22 + m_channels * 120), 1, QColor(136, 153, 187));
    painter->fillRect(3, (height / 2) + 2, (22 + m_channels * 120), 1, QColor(17, 51, 85));

    for (unsigned int chan = 0; chan < m_channels; chan++)
    {
        //channel dividers
        pen.setColor(QColor(170, 204, 238));
        painter->setPen(pen);
        painter->drawLine((24 + chan * 120), 0, (24 + chan * 120), height - 5);
    }
}

void HivelyTrackerPatternView::paintAbove(QPainter* painter, int height, int currentRow)
{
    QColor colorBase(68, 102, 136);
    QColor colorHilite(136, 153, 187);
    QColor colorShadow(17, 51, 85);
    QColor colorAntiAlias(51, 85, 119);

    QPen pen(colorShadow);
    pen.setWidth(1);
    painter->setPen(pen);


    //left frame
    painter->fillRect(0, 104, 1, (height) - 2, colorBase);
    QLinearGradient gradient1(QPointF(0, 0), QPointF(0, (height) - 2));
    gradient1.setColorAt(0, QColor(255, 255, 255).rgb());
    gradient1.setColorAt(0.5, colorHilite.rgb());
    painter->fillRect(1, 104, 1, (height) - 2, QBrush(gradient1));
    painter->fillRect(2, 105, 1, (height) - 4, QColor(34, 68, 102));
    painter->fillRect(2, 104, 1, 1, colorAntiAlias);


    //anti alias
    painter->fillRect(0, (height) - 2, 1, 1, colorAntiAlias);
    painter->fillRect(2, (height) - 4, 1, 1, colorAntiAlias);

    //right frame
    QLinearGradient gradient7(QPointF(0, 0), QPointF(0, (height) - 2));
    gradient7.setColorAt(0.5, colorHilite);
    gradient7.setColorAt(1, QColor(238, 245, 252).rgb());
    painter->fillRect((23 + m_channels * 120), 104, 1, (height) - 2, QBrush(gradient7));
    painter->fillRect((24 + m_channels * 120), 104, 1, (height) - 2, colorBase);
    QLinearGradient gradient2(QPointF(0, 0), QPointF(0, (height) - 2));
    gradient2.setColorAt(0.5, QColor(151, 175, 209).rgb());
    gradient2.setColorAt(1, QColor(238, 245, 252).rgb());
    painter->fillRect((25 + m_channels * 120), 104, 1, (height) - 2, QBrush(gradient2));
    QLinearGradient gradient3(QPointF(0, 0), QPointF(0, (height) - 1));
    gradient3.setColorAt(0, QColor(43, 76, 111).rgb());
    gradient3.setColorAt(1, QColor(17, 51, 85).rgb());
    painter->fillRect((26 + m_channels * 120), 104, 1, (height - 1), QBrush(gradient3));
    QLinearGradient gradient5(QPointF(0, 0), QPointF(0, (height) - 1));
    gradient5.setColorAt(0, QColor(26, 59, 93).rgb());
    gradient5.setColorAt(1, colorShadow);
    painter->fillRect((27 + m_channels * 120), 104, 1, (height - 1), QBrush(gradient5));

    //bottom frame
    painter->fillRect((3), (height) - 4, (20) + m_channels * 120, 1, colorBase);
    QLinearGradient gradient4(QPointF(0, 0), QPointF((25) + m_channels * 120, 0));
    gradient4.setColorAt(0.5, colorHilite);
    gradient4.setColorAt(1, QColor(255, 255, 255).rgb());
    painter->fillRect((1), (height) - 3, (25) + m_channels * 120, 1, QBrush(gradient4));
    painter->fillRect(1, (height) - 2, (25) + m_channels * 120, 1, colorShadow);
    QLinearGradient gradient6(QPointF(0, 0), QPointF((28) + m_channels * 120, 0));
    gradient6.setColorAt(0, QColor(73, 78, 78));
    gradient6.setColorAt(0.5, QColor(92, 106, 121).rgb());
    gradient6.setColorAt(1, QColor(82, 96, 108).rgb());
    painter->fillRect(0, (height) - 1, (28) + m_channels * 120, 1, QBrush(gradient6)); //gradient
}

void::HivelyTrackerPatternView::paintTop(QPainter* painter,Info* info, unsigned int m_currentPattern, unsigned int m_currentPosition, unsigned int m_currentSpeed, unsigned int m_currentBPM, unsigned int m_currentRow)
{
    m_topHeight = 104;
    int top = 2;
    int left = 0;
    QColor colorBase(170, 204, 238);
    QColor colorShadow(17, 51, 85);


    QRect rectBg(0, 0, m_width, m_topHeight);
    painter->fillRect(rectBg, QColor(0, 0, 0));


    QString imagepath = dataPath + RESOURCES_DIR +
                        QDir::separator() + "trackerview" + QDir::separator() + "hively_top.png";
    QImage imageTop(imagepath);


    QRectF sourceTrack(0, 0, 120, 25);

    painter->setFont(m_font);
    QPen pen(colorBase);
    for (int i = 0; i < info->modTrackPositions.size(); i++)
    {
        QRectF targetTrack(left + (15) + (i * 120), m_topHeight - 25, 120, 25);
        painter->drawImage(targetTrack, imageTop, sourceTrack);
        pen.setColor(colorBase);
        painter->setPen(pen);
        m_font.setLetterSpacing(QFont::AbsoluteSpacing, 0);
        painter->setFont(m_font);
        painter->drawText(left + (108) + (i * 120), m_topHeight - 8,
                          QString("%1").arg(info->modTrackPositions[i], 3, 10, QChar('0')));
        pen.setColor(colorShadow);
        painter->setPen(pen);
        m_font.setLetterSpacing(QFont::AbsoluteSpacing, -1);
        painter->setFont(m_font);
        painter->drawText(left + (25) + (i * 120), m_topHeight - 7, "Track " + QString::number(i + 1));
        pen.setColor(QColor(255, 255, 255));
        painter->setPen(pen);
        painter->drawText(left + (24) + (i * 120), m_topHeight - 8, "Track " + QString::number(i + 1));
    }
    m_font.setLetterSpacing(QFont::AbsoluteSpacing, 0);

    QFont fontsmall = m_font;
    fontsmall.setStyleStrategy(QFont::NoAntialias);

    //song square
    QRectF sourceSongs(166, 0, 337, 78);
    QRectF targetSongs(left + 265, 0, 337, 78);
    painter->drawImage(targetSongs, imageTop, sourceSongs);

    //pattern, position etc. square
    QRectF sourceInfo(503, 0, 263, 78);
    QRectF targetInfo(left, 0, 263, 78);
    painter->drawImage(targetInfo, imageTop, sourceInfo);


    fontsmall.setPixelSize(13);
    painter->setFont(fontsmall);
    pen.setColor(colorShadow);
    painter->setPen(pen);
    painter->drawText(left + 8, top + (24), "Position");
    pen.setColor(QColor(255, 255, 255));
    painter->setPen(pen);
    painter->drawText(left + 7, top + (23), "Position");

    pen.setColor(colorShadow);
    painter->setPen(pen);
    painter->drawText(left + 8, top + (44), "Length");
    pen.setColor(QColor(255, 255, 255));
    painter->setPen(pen);
    painter->drawText(left + 7, top + (43), "Length");

    pen.setColor(colorShadow);
    painter->setPen(pen);
    painter->drawText(left + 8, top + (64), "Restart");
    pen.setColor(QColor(255, 255, 255));
    painter->setPen(pen);
    painter->drawText(left + 7, top + (63), "Restart");

    pen.setColor(colorShadow);
    painter->setPen(pen);
    painter->drawText(left + 130, top + (24), "Track Len");
    pen.setColor(QColor(255, 255, 255));
    painter->setPen(pen);
    painter->drawText(left + 129, top + (23), "Track Len");

    pen.setColor(colorShadow);
    painter->setPen(pen);
    painter->drawText(left + 130, top + (44), "Subsongs");
    pen.setColor(QColor(255, 255, 255));
    painter->setPen(pen);
    painter->drawText(left + 129, top + (43), "Subsongs");


    painter->setFont(m_font);
    pen.setColor(colorBase);
    painter->setPen(pen);
    painter->drawText(left + (96), top + (24), QString("%1").arg(m_currentPosition, 3, 10, QChar('0')));
    painter->drawText(left + (96), top + (44), QString("%1").arg(info->numOrders, 3, 10, QChar('0')));
    painter->drawText(left + (96), top + (64),
                      QString("%1").arg(info->modPatternRestart, 3, 10, QChar('0')));
    painter->drawText(left + (217), top + (24),
                      QString("%1").arg(info->modPatternRows, 3, 10, QChar('0')));
    painter->drawText(left + (217), top + (44), QString("%1").arg(info->numSubsongs, 3, 10, QChar('0')));
    painter->drawText(left + (275), top + (43), QString(info->title.c_str()));

    //far left frame
    QRectF sourceLeftFrameSource(0, 25, 25, 25);
    QRectF targetLeftFrameTarget(left, 79, 25, 25);
    painter->drawImage(targetLeftFrameTarget, imageTop, sourceLeftFrameSource);

    //right frame
    QRectF rightFrameSource(18, 25, 13, 25);
    QRectF rightFrameTarget(left + info->modTrackPositions.size() * 120 + (15), 79, 13, 25);
    painter->drawImage(rightFrameTarget, imageTop, rightFrameSource);
}
HivelyTrackerPatternView::~HivelyTrackerPatternView()
{
}
