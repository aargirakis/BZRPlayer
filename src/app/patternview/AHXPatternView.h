#ifndef AHXPATTERNVIEW_H
#define AHXPATTERNVIEW_H
#include "AbstractPatternView.h"
#include "info.h"

class AHXPatternView : public AbstractPatternView
{
public:
    AHXPatternView(Tracker* parent, unsigned int channels, int scale);
    BitmapFont infoFont();
    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);
    void paintTop(QPainter* painter,Info* info, unsigned int m_currentPattern, unsigned int m_currentPosition, unsigned int m_currentSpeed, unsigned int m_currentBPM, unsigned int m_currentRow);
    ~AHXPatternView();

private:
};

#endif // AHXPATTERNVIEW_H
