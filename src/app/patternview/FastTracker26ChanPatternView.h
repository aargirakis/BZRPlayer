#ifndef FASTTRACKER26CHANPATTERNVIEW_H
#define FASTTRACKER26CHANPATTERNVIEW_H
#include "FastTracker2PatternView.h"

class FastTracker26ChanPatternView : public FastTracker2PatternView
{

public:
    FastTracker26ChanPatternView(Tracker *parent, unsigned int channels, int scale);
    ~FastTracker26ChanPatternView();
    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);

private:

protected:


};

#endif // FASTTRACKER26CHANPATTERNVIEW_H
