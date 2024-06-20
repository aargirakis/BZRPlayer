#include "FastTracker24ChanPatternView.h"

FastTracker24ChanPatternView::FastTracker24ChanPatternView(Tracker *parent, unsigned int channels, int scale)
                            :FastTracker2PatternView(parent,channels,scale)
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
    m_fontRownumber = QFont("Fasttracker 2");
    m_bitmapFontRownumber = BitmapFont("Fasttracker 2");
    m_fontRownumber.setPixelSize(8);
    m_fontWidthRownumber = 8;
    m_fontWidthEffects = 8;
    m_fontWidthInstrument = 8;
    m_fontWidthEffects = 8;
    m_fontWidthParameters = 8;
    m_fontWidthSeparatorNote = 8;
    m_SeparatorNote="''";
    m_RowLength = 40;

    m_xChannelStart = 28;
    m_channelWidth = 142;
    m_channelxSpace = 2;
}

FastTracker24ChanPatternView::~FastTracker24ChanPatternView()
{

}
int FastTracker24ChanPatternView::fontWidthRownumber()
{
    return m_fontWidthRownumber;
}
QFont FastTracker24ChanPatternView::fontRownumber()
{
    return m_fontRownumber;
}
BitmapFont FastTracker24ChanPatternView::bitmapFontRownumber()
{
    return m_bitmapFontRownumber;
}
