#ifndef FASTTRACKER1PATTERNVIEW_H
#define FASTTRACKER1PATTERNVIEW_H
#include "AbstractPatternView.h"

class FastTracker1PatternView : public AbstractPatternView
{
public:
    FastTracker1PatternView(Tracker* parent, unsigned int channels);
    ~FastTracker1PatternView();
    void paintBelow(QPainter* painter, int height, int currentRow);
    void paintTop(QPainter* painter,Info* info, unsigned int m_currentPattern, unsigned int m_currentPosition, unsigned int m_currentSpeed, unsigned int m_currentBPM, unsigned int m_currentRow);

private:

private slots:
};

#endif // FASTTRACKER1PATTERNVIEW_H
