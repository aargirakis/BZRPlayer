#ifndef MEDPATTERNVIEW_H
#define MEDPATTERNVIEW_H
#include "AbstractPatternView.h"

class MEDPatternView : public AbstractPatternView
{
public:
    MEDPatternView(Tracker* parent, unsigned int channels, int scale);
    ~MEDPatternView();
    BitmapFont infoFont();
    QString effect(BaseRow* row);
    QString note(BaseRow* row);
    QString instrument(BaseRow* row);
    QString rowNumber(int rowNumber);
    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);
    void paintTop(QPainter* painter,Info* info, unsigned int m_currentPattern, unsigned int m_currentPosition, unsigned int m_currentSpeed, unsigned int m_currentBPM, unsigned int m_currentRow);

private:

private slots:
};

#endif // MEDPATTERNVIEW_H
