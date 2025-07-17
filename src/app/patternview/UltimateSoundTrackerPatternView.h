#ifndef ULTIMATESOUNDTRACKERPATTERNVIEW_H
#define ULTIMATESOUNDTRACKERPATTERNVIEW_H
#include "AbstractPatternView.h"

class UltimateSoundTrackerPatternView : public AbstractPatternView
{
public:
    UltimateSoundTrackerPatternView(Tracker* parent, unsigned int channels, int scale);
    ~UltimateSoundTrackerPatternView();
    QFont currentRowFont();
    BitmapFont currentRowBitmapFont();
    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);
    void paintTop(QPainter* painter,Info* info, unsigned int m_currentPattern, unsigned int m_currentPosition, unsigned int m_currentSpeed, unsigned int m_currentBPM, unsigned int m_currentRow);

private:

private slots:
};

#endif // ULTIMATESOUNDTRACKERPATTERNVIEW_H
