#ifndef DIGIBOOSTER17PATTERNVIEW_H
#define DIGIBOOSTER17PATTERNVIEW_H
#include "AbstractPatternView.h"

class DigiBooster17PatternView : public AbstractPatternView
{

public:
    DigiBooster17PatternView(Tracker *parent, unsigned int channels, int scale);
    ~DigiBooster17PatternView();
    QFont currentRowFont();
    BitmapFont currentRowBitmapFont();
    BitmapFont infoFont();
    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);



private:


private slots:

};

#endif // DIGIBOOSTER17PATTERNVIEW_H
