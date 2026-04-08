#ifndef TRACKER_H
#define TRACKER_H

#include "patternview/AbstractPatternView.h"

class Tracker {
public:
    Tracker();

    void paint(QPainter *painter, QPaintEvent *event);

    void drawPattern(QPainter *painter, int visibleWidth, bool forceRedraw = false);

    void drawVuMeters(QPainter *painter) const;

    void drawTop(QPainter *painter) const;

    float scale;
    int m_visibleWidth;

    void init();

    Info *info;
    AbstractPatternView *trackerView;
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

    unsigned int m_currentSample;

    QPen m_pen;

    vector<unsigned char> m_currentTrackPositions;

    BitmapFont m_font;
};

#endif // TRACKER_H
