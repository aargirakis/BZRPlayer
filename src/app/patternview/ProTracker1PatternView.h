#ifndef PROTRACKER1PATTERNVIEW_H
#define PROTRACKER1PATTERNVIEW_H
#include "AbstractPatternView.h"

class ProTracker1PatternView : public AbstractPatternView
{
public:
    ProTracker1PatternView(Tracker* parent, unsigned int channels);
    ~ProTracker1PatternView();
    QFont currentRowFont();
    BitmapFont currentRowBitmapFont();
    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);
    void paintTop(QPainter* painter,Info* info, unsigned int m_currentPattern, unsigned int m_currentPosition, unsigned int m_currentSpeed, unsigned int m_currentBPM, unsigned int m_currentRow);

private:

private slots:
};

#endif // PROTRACKER1PATTERNVIEW_H
