#ifndef SCREAMTRACKER3PATTERNVIEW_H
#define SCREAMTRACKER3PATTERNVIEW_H
#include "AbstractPatternView.h"

class ScreamTracker3PatternView : public AbstractPatternView
{

public:
    ScreamTracker3PatternView(Tracker *parent, unsigned int channels, int scale);
    ~ScreamTracker3PatternView();
    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);
    QString effect(BaseRow* row);
    QString note(BaseRow *row);


private:

private slots:

};

#endif // SCREAMTRACKER3PATTERNVIEW_H
