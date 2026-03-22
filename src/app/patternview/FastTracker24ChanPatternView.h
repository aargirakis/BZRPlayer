#ifndef FASTTRACKER24CHANPATTERNVIEW_H
#define FASTTRACKER24CHANPATTERNVIEW_H

#include "FastTracker2PatternView.h"

class FastTracker24ChanPatternView : public FastTracker2PatternView
{
public:
    FastTracker24ChanPatternView(Tracker* parent, unsigned int channels);
    ~FastTracker24ChanPatternView();
    int fontWidthRowNumber();
    QFont fontRowNumber();
    BitmapFont bitmapFontRowNumber();
};

#endif // FASTTRACKER24CHANPATTERNVIEW_H
