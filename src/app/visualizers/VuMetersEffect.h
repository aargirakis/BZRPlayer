#ifndef BZR2_VUMETERSEFFECT_H
#define BZR2_VUMETERSEFFECT_H

#include <QPainter>

class VuMetersEffect {
public:
    VuMetersEffect();

    // settings
    void setEnabled(const bool on) { m_enabled = on; }
    bool enabled() const { return m_enabled; }
    void setTopColor(const QColor &c) { m_colTop = c; }
    void setMiddleColor(const QColor &c) { m_colMid = c; }
    void setBottomColor(const QColor &c) { m_colBot = c; }
    void setPeakColor(const QColor &c) { m_colPeak = c; }
    QColor topColor() const { return m_colTop; }
    QColor middleColor() const { return m_colMid; }
    QColor bottomColor() const { return m_colBot; }
    QColor peakColor() const { return m_colPeak; }
    void setOpacityPercent(const int pct) { m_opacity = std::clamp(pct, 0, 100) / 100.0; }
    int opacityPercent() const { return static_cast<int>(m_opacity * 100.0 + 0.5); }
    void setBarWidthPercent(const double pct) { m_widthPct = pct; }
    double barWidthPercent() const { return m_widthPct; }
    void setPeaksEnabled(const bool on) { m_peaksEnabled = on; }
    bool peaksEnabled() const { return m_peaksEnabled; }
    void setPeakHeight(const int px) { m_peakHeight = std::max(0, px); }
    int peakHeight() const { return m_peakHeight; }
    // end settings

    // reflection
    void setReflectionEnabled(const bool on) { m_reflectionEnabled = on; }
    bool reflectionEnabled() const { return m_reflectionEnabled; }
    void setReflectionOpacity(const double o01) { m_reflectionOpacity = std::clamp(o01, 0.0, 1.0); }
    int reflectionOpacityPercent() const { return static_cast<int>(m_reflectionOpacity * 100.0 + 0.5); }

    void setCanvasSize(const int w, const int h) {
        canvasWidth = w;
        canvasHeight = h;
    }

    void setAvailableHeight(const int h) { m_maxHeightOverride = h; }

    void paint(QPainter *painter, bool stereo);

private:
    // settings
    bool m_enabled = true;
    QColor m_colTop{0, 255, 64};
    QColor m_colMid{64, 255, 128};
    QColor m_colBot{0, 128, 32};
    QColor m_colPeak{255, 255, 255};
    double m_opacity = 1.0;
    double m_widthPct = 100.0;
    bool m_peaksEnabled = true;
    int m_peakHeight = 6;
    bool m_reflectionEnabled = true;
    double m_reflectionOpacity = 0.04;
    // end settings

    struct Peak {
        int currentY = 0;
        int timeLeftStill = 0;
        int currentSpeedDrop = 0;

        Peak(const int y = 0) : currentY(y) {
        }
    };

    std::vector<Peak> m_peaks;

    void ensurePeakSlots(size_t n);

    int canvasWidth = 320, canvasHeight = 256;
    int m_maxHeightOverride = -1;
    int m_numChannels;
};

#endif //BZR2_VUMETERSEFFECT_H
