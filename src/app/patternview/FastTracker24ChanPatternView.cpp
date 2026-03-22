#include "FastTracker24ChanPatternView.h"

FastTracker24ChanPatternView::FastTracker24ChanPatternView(Tracker* parent, const unsigned int channels)
    : FastTracker2PatternView(parent, channels)
{
    m_font = QFont("Fasttracker 2 16px");
    m_font.setPixelSize(8);
    m_fontWidth = 16;

    m_bitmapFont = BitmapFont("Fasttracker 2 16px");
    m_fontWidth = 16;
    m_fontHeight = 8;

    m_font2 = QFont("Fasttracker 2");
    m_fontEffects = QFont("Fasttracker 2");
    m_bitmapFontEffects = BitmapFont("Fasttracker 2");
    m_fontEffects.setPixelSize(8);
    m_fontParameters = QFont("Fasttracker 2");
    m_bitmapFontParameters = BitmapFont("Fasttracker 2");
    m_bitmapFontInstrument = BitmapFont("Fasttracker 2");
    m_fontParameters.setPixelSize(8);
    m_fontWidthEffects = 8;
    m_fontRowNumber = QFont("Fasttracker 2");
    m_bitmapFontRowNumber = BitmapFont("Fasttracker 2");
    m_fontRowNumber.setPixelSize(8);
    m_fontWidthRowNumber = 8;
    m_fontWidthEffects = 8;
    m_fontWidthInstrument = 8;
    m_fontWidthEffects = 8;
    m_fontWidthParameters = 8;
    m_fontWidthSeparatorNote = 8;
    m_SeparatorNote = "''";
    m_RowLength = 40;

    m_xChannelStart = 28;
    m_channelWidth = 142;
    m_channelxSpace = 2;
}

FastTracker24ChanPatternView::~FastTracker24ChanPatternView()
{
}

int FastTracker24ChanPatternView::fontWidthRowNumber()
{
    return m_fontWidthRowNumber;
}

QFont FastTracker24ChanPatternView::fontRowNumber()
{
    return m_fontRowNumber;
}

BitmapFont FastTracker24ChanPatternView::bitmapFontRowNumber()
{
    return m_bitmapFontRowNumber;
}
