#ifndef PROTRACKER1PATTERNVIEW_H
#define PROTRACKER1PATTERNVIEW_H
#include "AbstractPatternView.h"

class ProTracker1PatternView : public AbstractPatternView
{
public:
    ProTracker1PatternView(Tracker* parent, unsigned int channels, int scale);
    ~ProTracker1PatternView();
    QFont currentRowFont();
    BitmapFont currentRowBitmapFont();
    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);

private:

private slots:
};

#endif // PROTRACKER1PATTERNVIEW_H
