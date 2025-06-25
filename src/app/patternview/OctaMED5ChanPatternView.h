#ifndef OCTAMED5CHANPATTERNVIEW_H
#define OCTAMED5CHANPATTERNVIEW_H
#include "MEDPatternView.h"

class OctaMED5ChanPatternView : public MEDPatternView
{
public:
    OctaMED5ChanPatternView(Tracker* parent, unsigned int channels);
    ~OctaMED5ChanPatternView();
    QString effect(BaseRow* row);
    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);
    void paintTop(QPainter* painter,Info* info, unsigned int m_currentPattern, unsigned int m_currentPosition, unsigned int m_currentSpeed, unsigned int m_currentBPM, unsigned int m_currentRow);
    void drawVUMeters(QPainter* painter){};
private:

private slots:
};

#endif // OCTAMED5CHANPATTERNVIEW_H
