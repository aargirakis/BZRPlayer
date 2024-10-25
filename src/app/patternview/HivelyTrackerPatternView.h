#ifndef HIVELYTRACKERPATTERNVIEW_H
#define HIVELYTRACKERPATTERNVIEW_H
#include "AbstractPatternView.h"

class HivelyTrackerPatternView : public AbstractPatternView
{
public:
    HivelyTrackerPatternView(Tracker* parent, unsigned int channels, int scale);
    ~HivelyTrackerPatternView();
    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);

private:


private slots:
};

#endif // HIVELYTRACKERPATTERNVIEW_H
