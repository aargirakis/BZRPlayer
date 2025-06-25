#ifndef SOUNDTRACKERPATTERNVIEW_H
#define SOUNDTRACKERPATTERNVIEW_H
#include "NoiseTrackerPatternView.h"

class SoundTracker26PatternView : public NoiseTrackerPatternView
{
public:
    SoundTracker26PatternView(Tracker* parent, unsigned int channels);
    ~SoundTracker26PatternView();
    void paintBelow(QPainter* painter, int height, int currentRow);
    void paintTop(QPainter* painter,Info* info, unsigned int m_currentPattern, unsigned int m_currentPosition, unsigned int m_currentSpeed, unsigned int m_currentBPM, unsigned int m_currentRow);

private:

protected:
};

#endif // SOUNDTRACKERPATTERNVIEW_H
