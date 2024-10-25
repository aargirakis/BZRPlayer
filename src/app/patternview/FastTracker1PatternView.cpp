#include "FastTracker1PatternView.h"

FastTracker1PatternView::FastTracker1PatternView(Tracker* parent, unsigned int channels, int scale)
    : AbstractPatternView(parent, channels, scale)
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

FastTracker1PatternView::~FastTracker1PatternView()
{
}
