#ifndef ULTRATRACKERPATTERNVIEW_H
#define ULTRATRACKERPATTERNVIEW_H
#include "AbstractPatternView.h"

class UltraTrackerPatternView : public AbstractPatternView
{
public:
    UltraTrackerPatternView(Tracker* parent, unsigned int channels);
    ~UltraTrackerPatternView();
    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);
    QString parameter(BaseRow* row);
    QString parameter2(BaseRow* row);
    QString note(BaseRow* row);

private:


private slots:
};

#endif // ULTRATRACKERPATTERNVIEW_H
