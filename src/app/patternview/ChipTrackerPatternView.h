#ifndef CHIPTRACKERPATTERNVIEW_H
#define CHIPTRACKERPATTERNVIEW_H
#include "AbstractPatternView.h"

class ChipTrackerPatternView : public AbstractPatternView
{
public:
    ChipTrackerPatternView(Tracker* parent, unsigned int channels, int scale);
    ~ChipTrackerPatternView();
    QFont currentRowFont();
    BitmapFont currentRowBitmapFont();
    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);

private:

private slots:
};

#endif // CHIPTRACKERPATTERNVIEW_H
