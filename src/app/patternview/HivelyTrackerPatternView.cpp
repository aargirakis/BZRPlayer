#include "HivelyTrackerPatternView.h"

HivelyTrackerPatternView::HivelyTrackerPatternView(Tracker* parent, unsigned int channels, int scale)
    : AbstractPatternView(parent, channels, scale)
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

HivelyTrackerPatternView::~HivelyTrackerPatternView()
{
}
