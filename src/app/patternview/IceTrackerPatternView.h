#ifndef ICETRACKERPATTERNVIEW_H
#define ICETRACKERPATTERNVIEW_H
#include "AbstractPatternView.h"

class IceTrackerPatternView : public AbstractPatternView
{
public:
    IceTrackerPatternView(Tracker* parent, unsigned int channels);
    ~IceTrackerPatternView();
    QFont currentRowFont();
    BitmapFont currentRowBitmapFont();
    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);
    void paintTop(QPainter* painter,Info* info, unsigned int m_currentPattern, unsigned int m_currentPosition, unsigned int m_currentSpeed, unsigned int m_currentBPM, unsigned int m_currentRow);

private:

private slots:
};

#endif // ICETRACKERPATTERNVIEW_H
