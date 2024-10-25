#ifndef FASTTRACKER1PATTERNVIEW_H
#define FASTTRACKER1PATTERNVIEW_H
#include "AbstractPatternView.h"

class FastTracker1PatternView : public AbstractPatternView
{
public:
    FastTracker1PatternView(Tracker* parent, unsigned int channels, int scale);
    ~FastTracker1PatternView();
    void paintBelow(QPainter* painter, int height, int currentRow);

private:

private slots:
};

#endif // FASTTRACKER1PATTERNVIEW_H
