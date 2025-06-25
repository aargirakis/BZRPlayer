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
    void drawText(QString, QPainter*, int, int, int letterSpacing = 0);
    void setFont(BitmapFont);
    void paint(QPainter* painter, QPaintEvent* event);
    void drawToPainter(QPainter* painter);
    bool m_render;
    int m_height;
    float m_scale;
    void init();
    int TOP_OFFSET;
    int maxHeight;
    Info* m_info;
    AbstractPatternView* m_trackerview;
    unsigned int m_currentPattern;
    unsigned int m_currentPosition;
    unsigned int m_currentSpeed;
    unsigned int m_currentBPM;
    unsigned int m_currentRow;

private:
    QPixmap m_backBuffer;
    QSize m_lastBufferSize;
    bool m_renderVUMeter;
    bool m_renderTop;

    QPen m_pen;
    float m_fColorHueCounter;
    float m_fColorLightnessCounter;
    float m_fColorLightnessdirection;
    float m_fColorSaturationCounter;
    float m_fColorSaturationdirection;

    void drawVerticalEmboss(int xPos, int yPos, int height, QColor hilite, QColor shadow, QColor base,
                            QPainter* painter, bool left = true, bool right = true);

    vector<unsigned char> m_currentTrackPositions;


    BitmapFont m_font;

    unsigned char m_cAHXFontSize = 1;
    unsigned char m_cChipTrackerFontSize = 1;
    unsigned char m_cComposer669FontSize = 1;
    unsigned char m_cDigiBoosterFontSize = 1;
    unsigned char m_cDigiBoosterProFontSize = 1;
    unsigned char m_cFasttracker1FontSize = 1;
    unsigned char m_cFasttracker2FontSize = 1;
    unsigned char m_cGMCFontSize = 1;
    unsigned char m_cHivelyFontSize = 3;
    unsigned char m_cIceTrackerFontSize = 1;
    unsigned char m_cImpulseTracker2FontSize = 1;
    unsigned char m_cMEDFontSize = 1;
    unsigned char m_cOctaMEDSSFontSize = 1;
    unsigned char m_cMultiTrackerFontSize = 1;
    unsigned char m_cOktalyzerFontSize = 1;
    unsigned char m_cProTrackerFontSize = 1;
    unsigned char m_cProTracker36FontSize = 1;
    unsigned char m_cScreamTracker2FontSize = 1;
    unsigned char m_cScreamTracker3FontSize = 1;
    unsigned char m_cSFXFontSize = 1;
    unsigned char m_cSoundTracker2FontSize = 1;
    unsigned char m_cSoundTracker26FontSize = 1;
    unsigned char m_cStarTrekkerFontSize = 1;
    unsigned char m_cTakeTrackerFontSize = 1;
    unsigned char m_cUltimateSoundTrackerFontSize = 1;
    unsigned char m_cUltraTrackerFontSize = 1;

    bool eventFilter(QObject* obj, QEvent* event);
};

#endif // TRACKER_H
