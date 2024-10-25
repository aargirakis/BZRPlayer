#ifndef NOISETRACKERPATTERNVIEW_H
#define NOISETRACKERPATTERNVIEW_H
#include "AbstractPatternView.h"

class NoiseTrackerPatternView : public AbstractPatternView
{
public:
    NoiseTrackerPatternView(Tracker* parent, unsigned int channels, int scale);
    ~NoiseTrackerPatternView();
    QFont currentRowFont();
    BitmapFont currentRowBitmapFont();
    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);

private:

private slots:
};

#endif // NOISETRACKERPATTERNVIEW_H
