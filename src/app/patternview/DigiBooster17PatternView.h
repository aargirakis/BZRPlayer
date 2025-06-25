#ifndef DIGIBOOSTER17PATTERNVIEW_H
#define DIGIBOOSTER17PATTERNVIEW_H
#include "AbstractPatternView.h"

class DigiBooster17PatternView : public AbstractPatternView
{
public:
    DigiBooster17PatternView(Tracker* parent, unsigned int channels);
    ~DigiBooster17PatternView();
    QFont currentRowFont();
    BitmapFont currentRowBitmapFont();
    BitmapFont infoFont();
    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);
    void paintTop(QPainter* painter,Info* info, unsigned int m_currentPattern, unsigned int m_currentPosition, unsigned int m_currentSpeed, unsigned int m_currentBPM, unsigned int m_currentRow);

private:


private slots:
};

#endif // DIGIBOOSTER17PATTERNVIEW_H
