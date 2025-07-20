#include "FastTracker1PatternView.h"

FastTracker1PatternView::FastTracker1PatternView(Tracker* parent, unsigned int channels)
    : AbstractPatternView(parent, channels)
{
    octaveOffset = 48;
    rowNumberOffset = 0;
    m_font = QFont("FastTracker 1.0");
    m_font.setPixelSize(16);


    m_bitmapFont = BitmapFont("FastTracker 1.0");
    m_fontWidth = 8;
    m_fontHeight = 16;

    m_font.setStyleStrategy(QFont::NoAntialias);
    m_RowEnd = m_SeparatorRowNumber = m_SeparatorChannel = "'";
    m_SeparatorRowNumber = "''";
    m_SeparatorNote = "'";
    m_SeparatorRowNumberLast = "''";
    m_rowNumbersLastChannelEnabled = true;
    m_RowLength = 11 + m_channels * 10;
    m_colorCurrentRowBackground = QColor(146, 146, 162);
    m_colorCurrentRowForeground = QColor(239, 239, 243);

    m_colorDefault = m_ColorRowNumber = m_ColorInstrument = m_ColorEffect = m_ColorParameter = m_ColorVolume =
        m_colorEmpty = QColor(80, 138, 255);
    m_xOffsetRow = 8;
    m_xChannelStart = 32;
    m_channelWidth = 80;
    m_channelxSpace = 0;
}

void FastTracker1PatternView::paintBelow(QPainter* painter, int height, int currentRow)
{
    QColor colorBase(146, 146, 162);
    QColor colorHilite(255, 255, 255);
    painter->fillRect((1), (height / 2) - 15, (80 * m_channels) + (79), 14, colorBase);
    QPen pen(colorHilite);
    pen.setWidth(1);
    painter->setPen(pen);
    painter->drawLine(0, ((height / 2) - 16), (80 * m_channels) + (79), ((height / 2) - 16));
    painter->drawLine(0, (height / 2) - 1, (80 * m_channels) + (79), (height / 2) - 1);
    painter->drawLine(0, ((height / 2) - 16), 0, ((height / 2) - 1));
    painter->drawLine((80 * m_channels) + (79), ((height / 2) - 16), (80 * m_channels) + (79), ((height / 2) - 1));
}

void::FastTracker1PatternView::paintTop(QPainter* painter,Info* info, unsigned int m_currentPattern, unsigned int m_currentPosition, unsigned int m_currentSpeed, unsigned int m_currentBPM, unsigned int m_currentRow)
{
    m_topHeight = 31;
    QColor colorBase(146, 146, 162);
    QColor colorHilite(255, 255, 255);
    int top = 16;
    int left = 0 + 12;
    QRect rectBg(0, 0, m_width, m_topHeight);
    painter->fillRect(rectBg, colorBase);

    painter->setPen(QColor(255, 255, 255));
    drawText(QString("%1").arg(m_currentPosition, 4, 10, QChar('0')), painter, left + 88, top + 0, infoFont());
    painter->setPen(QColor(255, 255, 255));
    drawText("POSITION", painter, left + 0, top + 0, infoFont());
    painter->setPen(QColor(255, 255, 255));
    drawText("PATTERN", painter, left + 140, top + 0, infoFont());
    painter->setPen(QColor(255, 255, 255));
    drawText(QString("%1").arg(m_currentPattern, 4, 10, QChar('0')), painter, left + (228), top + 0, infoFont());
    painter->setPen(QColor(255, 255, 255));
    drawText("LENGTH", painter, left + 280, top + 0, infoFont());
    painter->setPen(QColor(255, 255, 255));
    drawText(QString("%1").arg(info->numOrders, 4, 10, QChar('0')), painter, left + (368), top + 0, infoFont());
    painter->setPen(QColor(255, 255, 255));
    drawText("SONGNAME:", painter, left + 0, top + (15), infoFont());
    painter->setPen(QColor(255, 255, 255));
    drawText(QString(info->title.c_str()).toUpper(), painter, left + (80), top + (15), infoFont());

    m_pen.setWidth(1);

    m_pen.setColor(colorHilite);
    painter->setPen(m_pen);
    painter->drawLine(0, 15, m_width, 15);

    m_pen.setColor(colorHilite);
    painter->setPen(m_pen);
    painter->drawLine(0, 0, m_width, 0);

    m_pen.setColor(colorHilite);
    painter->setPen(m_pen);
    painter->drawLine(0, m_topHeight - 1, m_width - 1, m_topHeight - 1);

    m_pen.setColor(colorHilite);
    painter->setPen(m_pen);
    painter->drawLine(1, 0, 1, m_topHeight - 1);

    m_pen.setColor(colorHilite);
    painter->setPen(m_pen);
    painter->drawLine(m_width, 0, m_width, m_topHeight - 1);

    m_pen.setColor(colorHilite);
    painter->setPen(m_pen);
    painter->drawLine(left + (129), 0, left + (129), 15);

    m_pen.setColor(colorHilite);
    painter->setPen(m_pen);
    painter->drawLine(left + (269), 0, left + (269), 15);

    m_pen.setColor(colorHilite);
    painter->setPen(m_pen);
    painter->drawLine(left + (409), 0, left + (409), m_topHeight - 1);
}
FastTracker1PatternView::~FastTracker1PatternView()
{
}
