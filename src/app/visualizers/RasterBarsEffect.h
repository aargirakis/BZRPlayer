//
// Created by Andreas on 2025-09-16.
//

#ifndef BZR2_RASTERBARSEFFECT_H
#define BZR2_RASTERBARSEFFECT_H

#include <QPainter>

class RasterBarsEffect {
public:
    RasterBarsEffect();

    //settings
    void setEnabled(bool on) { m_enabled = on; }
    bool enabled() const { return m_enabled; }
    void setCount(int n);
    int count() const { return m_count; }
    void setBarHeight(int px);
    int barHeight() const { return m_barHeight; }
    void setVerticalSpacing(int amount);
    int verticalSpacing() const { return 33 - m_verticalSpacing; }
    void setSpeed(int amount)  { m_speed = amount; }
    int speed() const { return m_speed; }
    void setOpacity(int percent);
    int opacity() const  { return int(m_opacity * 100.0); }
    //end settings

    void setCanvasSize(int w, int h) { m_width = w; m_height = h; }
    void paint(QPainter* painter);

private:
    void rebuildColors();
    void drawOneBar(QPainter* painter, int offset);

    //settings
    bool m_enabled;
    int m_count = 64;
    int m_barHeight;
    int m_verticalSpacing;
    int m_speed;
    double m_opacity;
    //end settings

    double m_counter = 0.0;
    double m_sineAngle = 0.0;
    int m_width;
    int m_height;
    std::vector<QBrush> m_barBrushes;
};


#endif //BZR2_RASTERBARSEFFECT_H
