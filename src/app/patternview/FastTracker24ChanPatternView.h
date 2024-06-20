#ifndef FASTTRACKER24CHANPATTERNVIEW_H
#define FASTTRACKER24CHANPATTERNVIEW_H
#include "FastTracker2PatternView.h"

class FastTracker24ChanPatternView : public FastTracker2PatternView
{

public:
    FastTracker24ChanPatternView(Tracker *parent, unsigned int channels, int scale);
    ~FastTracker24ChanPatternView();
    int fontWidthRownumber();
    QFont fontRownumber();
    BitmapFont bitmapFontRownumber();

private:

protected:


};

#endif // FASTTRACKER24CHANPATTERNVIEW_H
