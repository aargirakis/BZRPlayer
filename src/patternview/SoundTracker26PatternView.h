#ifndef SOUNDTRACKERPATTERNVIEW_H
#define SOUNDTRACKERPATTERNVIEW_H
#include "NoiseTrackerPatternView.h"

class SoundTracker26PatternView : public NoiseTrackerPatternView
{

public:
    SoundTracker26PatternView(Tracker *parent, unsigned int channels, int scale);
    ~SoundTracker26PatternView();
    void paintBelow(QPainter* painter, int height, int currentRow);

private:

protected:


};

#endif // SOUNDTRACKERPATTERNVIEW_H
