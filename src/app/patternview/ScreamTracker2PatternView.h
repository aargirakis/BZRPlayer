#ifndef SCREAMTRACKER2PATTERNVIEW_H
#define SCREAMTRACKER2PATTERNVIEW_H
#include "AbstractPatternView.h"

class ScreamTracker2PatternView : public AbstractPatternView
{

public:
    ScreamTracker2PatternView(Tracker *parent, unsigned int channels, int scale);
    ~ScreamTracker2PatternView();
    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);
    QString effect(BaseRow* row);
    QString note(BaseRow* row);
    QString parameter(BaseRow* row);


private:


private slots:

};

#endif // SCREAMTRACKER2PATTERNVIEW_H
