#ifndef MULTITRACKERPATTERNVIEW_H
#define MULTITRACKERPATTERNVIEW_H
#include "AbstractPatternView.h"

class MultiTrackerPatternView : public AbstractPatternView
{
public:
    MultiTrackerPatternView(Tracker* parent, unsigned int channels, int scale);
    ~MultiTrackerPatternView();
    QString note(BaseRow* row);
    QString rowNumber(int rowNumber);
    QString effect(BaseRow* row);
    QString parameter(BaseRow* row);
    QString instrument(BaseRow* row);

    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);

private:


private slots:
};

#endif // MULTITRACKERPATTERNVIEW_H
