#ifndef MULTITRACKERPATTERNVIEW_H
#define MULTITRACKERPATTERNVIEW_H
#include "AbstractPatternView.h"

class MultiTrackerPatternView : public AbstractPatternView
{
public:
    MultiTrackerPatternView(Tracker* parent, unsigned int channels);
    ~MultiTrackerPatternView();
    QString note(BaseRow* row);
    QString rowNumber(int rowNumber);
    QString effect(BaseRow* row);
    QString parameter(BaseRow* row);
    QString instrument(BaseRow* row);

    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);
    void paintTop(QPainter* painter,Info* info, unsigned int m_currentPattern, unsigned int m_currentPosition, unsigned int m_currentSpeed, unsigned int m_currentBPM, unsigned int m_currentRow);

private:


private slots:
};

#endif // MULTITRACKERPATTERNVIEW_H
