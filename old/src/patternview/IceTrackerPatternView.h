#ifndef ICETRACKERPATTERNVIEW_H
#define ICETRACKERPATTERNVIEW_H
#include "AbstractPatternView.h"

class IceTrackerPatternView : public AbstractPatternView
{

public:
    IceTrackerPatternView(Tracker *parent, unsigned int channels, int scale);
    ~IceTrackerPatternView();
    QFont currentRowFont();
    BitmapFont currentRowBitmapFont();
    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);


private:

private slots:

};

#endif // ICETRACKERPATTERNVIEW_H
