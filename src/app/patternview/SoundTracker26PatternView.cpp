#include "SoundTracker26PatternView.h"

SoundTracker26PatternView::SoundTracker26PatternView(Tracker *parent, unsigned int channels, int scale)
                            :NoiseTrackerPatternView(parent,channels,scale)
{
    m_ibuttonPrevSampleWidth=11;
    m_ibuttonPrevSampleHeight=11;
    m_ibuttonPrevSampleX=11;
    m_ibuttonPrevSampleY=22;
    m_ibuttonNextSampleWidth=11;
    m_ibuttonNextSampleHeight=11;
    m_ibuttonNextSampleX=0;
    m_ibuttonNextSampleY=22;
}

SoundTracker26PatternView::~SoundTracker26PatternView()
{

}
void SoundTracker26PatternView::paintBelow(QPainter* painter, int height, int currentRow)
{
    QColor colorHilite(173,170,173);
    NoiseTrackerPatternView::paintBelow(painter,height,currentRow);
    painter->fillRect(2,(height/2)-6,1,2,colorHilite);
}
