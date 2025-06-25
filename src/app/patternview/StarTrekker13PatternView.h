#ifndef STARTREKKER13PATTERNVIEW_H
#define STARTREKKER13PATTERNVIEW_H
#include "AbstractPatternView.h"

class StarTrekker13PatternView : public AbstractPatternView
{
public:
    StarTrekker13PatternView(Tracker* parent, unsigned int channels);
    ~StarTrekker13PatternView();
    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);
    void paintTop(QPainter* painter,Info* info, unsigned int m_currentPattern, unsigned int m_currentPosition, unsigned int m_currentSpeed, unsigned int m_currentBPM, unsigned int m_currentRow);
    QFont currentRowFont();
    BitmapFont currentRowBitmapFont();

private:

private slots:
};

#endif // STARTREKKER13PATTERNVIEW_H
