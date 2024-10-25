#ifndef PROTRACKER36PATTERNVIEW_H
#define PROTRACKER36PATTERNVIEW_H
#include "AbstractPatternView.h"

class ProTracker36PatternView : public AbstractPatternView
{
public:
    ProTracker36PatternView(Tracker* parent, unsigned int channels, int scale);
    ~ProTracker36PatternView();
    BitmapFont infoFont();
    BitmapFont infoFont2();
    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);

private:

private slots:
};

#endif // PROTRACKER36PATTERNVIEW_H
