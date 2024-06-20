#ifndef AHXPATTERNVIEW_H
#define AHXPATTERNVIEW_H
#include "AbstractPatternView.h"

class AHXPatternView : public AbstractPatternView
{

public:
    AHXPatternView(Tracker *parent, unsigned int channels, int scale);
    BitmapFont infoFont();
    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);
    ~AHXPatternView();


private:

};

#endif // AHXPATTERNVIEW_H
