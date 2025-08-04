#ifndef TRACKER_H
#define TRACKER_H

#include <QPaintEvent>
#include "info.h"
#include "patternview/AbstractPatternView.h"
#include "patternview/bitmapfont.h"

class Tracker
{
public:
    Tracker();

    void paint(QPainter* painter, QPaintEvent* event);
    void drawPattern(QPainter* painter);
    void drawVUMeters(QPainter* painter);
    void drawTop(QPainter* painter);
    float m_scale;
    void init();
    Info* m_info;
    AbstractPatternView* m_trackerview;
    unsigned int m_currentPattern;
    unsigned int m_currentPosition;
    unsigned int m_currentSpeed;
    unsigned int m_currentBPM;
    unsigned int m_currentRow;

    unsigned int m_currentPatternBuffer;
    unsigned int m_currentPositionBuffer;
    unsigned int m_currentSpeedBuffer;
    unsigned int m_currentBPMBuffer;
    unsigned int m_currentRowBuffer;
    unsigned int m_lastSample;


private:

    QPixmap m_vuBuffer;
    QSize m_lastVuSize;
    QPixmap m_backBuffer;
    QPixmap m_muteBuffer;
    QSize m_lastBufferSize;
    QPixmap m_topBarBuffer;
    QSize m_lastTopBarSize;

    QSize m_lastMuteSize;
    std::vector<bool> m_prevMuteState;


    unsigned  int m_currentSample;

    QPen m_pen;


    void drawVerticalEmboss(int xPos, int yPos, int height, QColor hilite, QColor shadow, QColor base,
                            QPainter* painter, bool left = true, bool right = true);

    vector<unsigned char> m_currentTrackPositions;


    BitmapFont m_font;


    bool eventFilter(QObject* obj, QEvent* event);
};

#endif // TRACKER_H
