#ifndef IMPULSETRACKERPATTERNVIEW_H
#define IMPULSETRACKERPATTERNVIEW_H
#include "AbstractPatternView.h"

class ImpulseTrackerPatternView : public AbstractPatternView
{

public:
    ImpulseTrackerPatternView(Tracker *parent, unsigned int channels, int scale);
    ~ImpulseTrackerPatternView();
    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);
    QString effect(BaseRow* row);
    QString note(BaseRow *row);
    QString rowNumber(int rowNumber);
    QString volume(BaseRow* row);


private:

private slots:

};

#endif // IMPULSETRACKERPATTERNVIEW_H
