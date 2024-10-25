#include "GenericPatternView.h"

GenericPatternView::GenericPatternView(Tracker* parent, unsigned int channels, int scale)
    : AbstractPatternView(parent, channels, scale)
{
    rowNumberOffset = 0;
    octaveOffset = 48;

    m_font = QFont("AHX Thin");
    m_font.setPixelSize(8 * m_scale);
    m_fontWidth = m_scale * 6;
    m_font.setStyleStrategy(QFont::NoAntialias);

    m_colorDefault = m_ColorRowNumber = m_ColorInstrument = m_ColorEffect = m_ColorParameter = m_ColorEffect2 =
        m_ColorParameter2 = m_ColorVolume = m_colorEmpty = QColor(255, 255, 255);
    m_colorCurrentRowForeground = QColor(255, 255, 0);
    m_RowEnd = m_SeparatorRowNumber = m_SeparatorChannel = " ";

    volumeEnabled = false;
    instrumentEnabled = false;
    effect2Enabled = false;
    effectEnabled = false;
    parameterEnabled = false;
    parameter2Enabled = false;
    m_emptyNote = " . ";
    m_RowLength = 3 + (m_channels * 4);
    m_xOffsetRow = 8;
}

void GenericPatternView::paintAbove(QPainter* painter, int height, int currentRow)
{
}

void GenericPatternView::paintBelow(QPainter* painter, int height, int currentRow)
{
    QColor colorBase(73, 117, 130);
    QColor colorHilite(138, 219, 243);
    QColor colorShadow(24, 40, 44);

    //main current row
    painter->fillRect(0, (height / 2) - 9, (52) + (m_channels * 72 * m_fontWidth / 8), 9, colorBase);
}

GenericPatternView::~GenericPatternView()
{
}
