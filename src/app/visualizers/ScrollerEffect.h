//
// Created by Andreas on 2025-09-17.
//

#ifndef BZR2_SCROLLEREFFECT_H
#define BZR2_SCROLLEREFFECT_H

#include <QPainter>
#include <QtCore/QString>
#include <vector>
#include <iostream>

class ScrollerEffect {
public:
    ScrollerEffect() = default;
    void setCanvasSize(int w, int h) { m_w = w; m_h = h; reinitForCanvas(); }
    void setScale(qreal sx, qreal sy) { m_scaleX = sx; m_scaleY = sy; }
    int getFontHeight() const  { return m_fontHeight; }
    int getBottomYMax() const { return m_bottomYMax; }
    void paint(QPainter* p, bool paused);
    //settings
    void setFontScaleX(int v);
    void setFontScaleY(int v);
    int getFontScaleX() const { return m_scaleXInt; }
    int getFontScaleY() const { return m_scaleYInt; }
    void setText(const QString& t);
    void setScrollSpeed(int pxPerFrame) { m_scrollSpeed = pxPerFrame;}
    int scrollSpeed() const { return m_scrollSpeed; }
    void setEnabled(bool on) { m_enabled = on; }
    bool enabled() const { return m_enabled; }
    bool setFont(const QString& pngPath);
    QString getFont() const { return m_fontPathPng; }
    QString text() const { return m_srcText; }
    void setAmplitude(int a) { m_amplitude = a; recomputeBottomYMax();}
    int amplitude() const {return m_amplitude;}
    void setSinusSpeed(double v) { m_sinusSpeed = v; }
    double sinusSpeed() const { return m_sinusSpeed; }
    void setSinusFrequency(double v) { m_sinusFreq = v; }
    double sinusFrequency() const { return m_sinusFreq; }
    void setSinusFontScalingEnabled(bool on) { m_sinusFontScaling = on; ; recomputeBottomYMax(); }
    bool sinusFontScalingEnabled() const { return m_sinusFontScaling; }
    void setVerticalScrollPosition(int v) { m_verticalScroll = v; }
    int getVerticalScrollPosition() const {return m_verticalScroll;}

    // reflection
    void setReflectionEnabled(bool on) { m_reflectionEnabled = on; }
    bool reflectionEnabled() const { return m_reflectionEnabled; }
    void setReflectionOpacity(double o01) { m_reflectionOpacity = o01; }
    double reflectionOpacity() const { return m_reflectionOpacity; }
    void setReflectionColor(const QColor& c) { m_reflectionColor = c; }
    QColor reflectionColor() const { return m_reflectionColor; }

private:
    //settings
    bool m_enabled = false;
    int m_scaleXInt = 1;
    int m_scaleYInt = 2;
    double m_sineAngle;
    int m_scrollSpeed;
    int m_amplitude;
    double m_sinusFreq;
    double m_sinusSpeed;
    bool m_sinusFontScaling;
    int  m_verticalScroll;
    QString m_fontPathPng;
    QString m_srcText;
    // font atlas & metrics

    QString m_charset;
    QPixmap m_atlas;
    int m_fontWOrig = 8, m_fontHOrig = 8;
    int m_fontWidth = 8, m_fontHeight = 16;
    int m_w = 320, m_h = 256;
    qreal  m_scaleX = 1.0, m_scaleY = 1.0;
    QString m_workText;
    int m_letters;
    int m_position;
    std::vector<int> m_chars;
    std::vector<int> m_x;
    int m_bottomYMax;

    // reflection
    bool m_reflectionEnabled = true;
    double m_reflectionOpacity ;
    QColor m_reflectionColor;

    // helpers
    void reinitForCanvas();
    void rebuildAtlas();
    void rebuildSlots();
    inline void recomputeBottomYMax() {
        m_bottomYMax = std::abs(m_amplitude) * m_scaleYInt;
    }
    QString normalizeText(QString t) const;
    bool loadBitmapFont(const QString& pngPath);
};



#endif //BZR2_SCROLLEREFFECT_H
