#ifndef DIGIBOOSTERPROPATTERNVIEW_H
#define DIGIBOOSTERPROPATTERNVIEW_H
#include "AbstractPatternView.h"

class DigiBoosterProPatternView : public AbstractPatternView
{

public:
    DigiBoosterProPatternView(Tracker *parent, unsigned int channels, int scale);
    ~DigiBoosterProPatternView();
    QFont currentRowFont();
    BitmapFont currentRowBitmapFont();
    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);
    QString rowNumber(int rowNumber);
    QString effect(BaseRow* row);
    QString effect2(BaseRow* row);


private:

private slots:

};

#endif // DIGIBOOSTERPROPATTERNVIEW_H
