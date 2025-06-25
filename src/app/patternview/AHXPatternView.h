#ifndef AHXPATTERNVIEW_H
#define AHXPATTERNVIEW_H
#include "AbstractPatternView.h"
#include "info.h"

class AHXPatternView : public AbstractPatternView
{
public:
    AHXPatternView(Tracker* parent, unsigned int channels);
    BitmapFont infoFont();
    void paintAbove(QPainter* painter, int height, int currentRow);
    void paintBelow(QPainter* painter, int height, int currentRow);
    void paintTop(QPainter* painter,Info* info, unsigned int m_currentPattern, unsigned int m_currentPosition, unsigned int m_currentSpeed, unsigned int m_currentBPM, unsigned int m_currentRow);
    void drawVUMeters(QPainter* painter);
    ~AHXPatternView();

private:
    float m_fColorHueCounter;
    float m_fColorLightnessCounter;
    float m_fColorLightnessdirection;
    float m_fColorSaturationCounter;
    float m_fColorSaturationdirection;
};

#endif // AHXPATTERNVIEW_H
