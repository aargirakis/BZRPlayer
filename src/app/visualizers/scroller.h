#ifndef SCROLLER_H
#define SCROLLER_H

#include "effect.h"
#include "PrinterEffect.h"
#include "RasterBarsEffect.h"
#include "RotatingObject.h"
#include "ScrollerEffect.h"
#include "StarField.h"
#include "VuMetersEffect.h"

class Scroller : public Effect {
public:
    Scroller(QWidget *parent);

    void paint(QPainter *painter, QPaintEvent *event);

    void setColorVisualizerBackground(const QColor c) { colorVisualizerBackground = c; }

    void setResolutionWidth(int);

    int getResolutionWidth() const override;

    void setResolutionHeight(int);

    int getResolutionHeight() const override;

    void setKeepAspectRatio(bool);

    bool getKeepAspectRatio() const override;

    QColor getColorVisualizerBackground() const override;

    QString getCustomScrolltext() const;

    void setCustomScrolltextEnabled(bool enabled);

    void setCustomScrolltext(QString text);

    // rasterbars
    void setRasterBarsEnabled(const bool enabled) { m_rasterBars->setEnabled(enabled); }
    bool getRasterBarsEnabled() const override { return m_rasterBars->enabled(); }
    void setNumberOfRasterBars(const int amount) { m_rasterBars->setCount(amount); }
    int getNumberOfRasterBars() const override { return m_rasterBars->count(); }
    void setRasterBarsBarHeight(const int amount) { m_rasterBars->setBarHeight(amount); }
    int getRasterBarsHeight() const override { return m_rasterBars->barHeight(); }
    void setRasterBarsVerticalSpacing(const int amount) { m_rasterBars->setVerticalSpacing(amount); }
    int getRasterBarsVerticalSpacing() const override { return m_rasterBars->verticalSpacing(); }
    void setRasterBarsSpeed(const int amount) { m_rasterBars->setSpeed(amount); }
    int getRasterBarsSpeed() const override { return m_rasterBars->speed(); }
    void setRasterBarsOpacity(const int amount) { m_rasterBars->setOpacity(amount); }
    int getRasterbarsOpacity() const override { return m_rasterBars->opacity(); }

    // stars
    void setStarsEnabled(const bool enabled) { m_starField->setEnabled(enabled); }
    bool getStarsEnabled() const override { return m_starField->enabled(); }
    void setNumberOfStars(const int n) { m_starField->setCount(n); }
    int getNumberOfStars() const override { return m_starField->count(); }
    void setStarSpeed(const int s) { m_starField->setSpeed(s); }
    int getStarSpeed() const override { return m_starField->speed(); }
    void setStarsDirection(const QString dir) { m_starField->setModeFromString(dir); }
    QString getStarsDirection() const override { return m_starField->modeToString(); }

    // scroller
    bool getCustomScrolltextEnabled() const override { return customScrolltextEnabled; }
    void setScrollerEnabled(const bool on) { m_scroller->setEnabled(on); }
    bool getScrollerEnabled() const override { return m_scroller->enabled(); }
    void setScrollText(const QString t) { m_scroller->setText(t); }
    void setScrollerFont(const QString fontPng) { m_scroller->setFont(fontPng); }
    QString getFont() const override { return m_scroller->getFont(); }
    void setFontScaleX(const int v) { m_scroller->setFontScaleX(v); }
    void setFontScaleY(const int v) { m_scroller->setFontScaleY(v); }
    int getFontScaleX() const override { return m_scroller->getFontScaleX(); }
    int getFontScaleY() const override { return m_scroller->getFontScaleY(); }
    void setScrollSpeed(const int v) { m_scroller->setScrollSpeed(v); }
    int getScrollSpeed() const override { return m_scroller->scrollSpeed(); }
    void setAmplitude(const int v) { m_scroller->setAmplitude(v); }
    int getAmplitude() const override { return m_scroller->amplitude(); }
    void setSinusSpeed(const double v) { m_scroller->setSinusSpeed(v); }
    double getSinusSpeed() const override { return m_scroller->sinusSpeed(); }
    void setSinusFrequency(const double v) { m_scroller->setSinusFrequency(v); }
    double getFrequency() const override { return m_scroller->sinusFrequency(); }
    void setSinusFontScalingEnabled(const bool on) { m_scroller->setSinusFontScalingEnabled(on); }
    bool getSinusFontScalingEnabled() const override { return m_scroller->sinusFontScalingEnabled(); }
    void setVerticalScrollPosition(const int v) { m_scroller->setVerticalScrollPosition(v); }
    int getVerticalScrollPosition() const override { return m_scroller->getVerticalScrollPosition(); }

    // rotating object
    bool getRotatingObjectEnabled() const override { return m_rotatingObject->enabled(); }
    void setRotatingObjectEnabled(const bool on) { m_rotatingObject->setEnabled(on); }
    bool getRotatingObjectWireframeEnabled() const override { return m_rotatingObject->wireframeEnabled(); }
    void setRotatingObjectWireframeEnabled(const bool on) { m_rotatingObject->setWireframeEnabled(on); }
    int getRotatingObjectFocalLength() const override { return m_rotatingObject->focalLength(); }
    void setRotatingObjectFocalLength(const int v) { m_rotatingObject->setFocalLength(v); }
    void setRotatingObjectOrbitSize(const int v) { m_rotatingObject->setOrbitSize(v); }
    void setRotatingObjectOrbitSpeed(const int v) { m_rotatingObject->setOrbitSpeed(v); }
    int getRotatingObjectOrbitSize() const override { return m_rotatingObject->orbitSize(); }
    int getRotatingObjectOrbitSpeed() const override { return m_rotatingObject->orbitSpeed(); }
    int getRotatingObjectSize() const override { return m_rotatingObject->size(); }

    void setRotatingObjectSize(const int v) {
        m_rotatingObject->setSize(v);
        m_rotatingObject->initModels();
    }

    QString getRotatingObjectColorWireframe() const override { return m_rotatingObject->wireColor().name(); }
    void setRotatingObjectColorWireframe(const QColor c) { m_rotatingObject->setWireColor(c); }
    QString getRotatingObjectColor() const override { return m_rotatingObject->fillColor().name(); }
    void setRotatingObjectColor(const QColor c) { m_rotatingObject->setFillColor(c); }
    bool getRotatingObjectOrbit() const override { return m_rotatingObject->orbitEnabled(); }
    void setRotatingObjectOrbit(const bool on) { m_rotatingObject->setOrbitEnabled(on); }
    QString getRotatingObjectModel() const override { return m_rotatingObject->model(); }
    void setRotatingObjectModel(const QString model) { m_rotatingObject->setModel(model); }
    QString getRotatingObjectMaterial() const override { return m_rotatingObject->material(); }
    void setRotatingObjectMaterial(const QString material) { m_rotatingObject->setMaterial(material); }

    // vu-meters
    void setVuMeterEnabled(const bool on) { if (m_vuMeters) m_vuMeters->setEnabled(on); }
    bool isVuMeterEnabled() const override { return m_vuMeters && m_vuMeters->enabled(); }
    void setVuMeterWidth(const double pct) { m_vuMeters->setBarWidthPercent(pct); }
    double getVuMeterWidth() const override { return m_vuMeters->barWidthPercent(); }
    void setVuMeterOpacity(const int pct) { m_vuMeters->setOpacityPercent(pct); }
    int getVuMeterOpacity() const override { return m_vuMeters->opacityPercent(); }
    void setVuMeterPeaksEnabled(const bool on) { if (m_vuMeters) m_vuMeters->setPeaksEnabled(on); }
    void setHeightVisualizerPeak(const int h) { if (m_vuMeters) m_vuMeters->setPeakHeight(h); }
    void setColorVisualizerTop(const QColor c) { if (m_vuMeters) m_vuMeters->setTopColor(c); }
    void setColorVisualizerMiddle(const QColor c) { if (m_vuMeters) m_vuMeters->setMiddleColor(c); }
    void setColorVisualizerBottom(const QColor c) { if (m_vuMeters) m_vuMeters->setBottomColor(c); }
    void setColorVisualizerPeakColor(const QColor c) { if (m_vuMeters) m_vuMeters->setPeakColor(c); }

    // printer
    void setPrinterEnabled(const bool on) { if (m_printer) m_printer->setEnabled(on); }
    bool getPrinterEnabled() const override { return m_printer && m_printer->enabled(); }
    void setPrinterFont(const QString path) { m_printer->setFont(path); }
    QString getPrinterFont() const override { return m_printer->getFont(); }
    void setPrinterFontScaleX(const int v) { if (m_printer) m_printer->setFontScaleX(v); }
    void setPrinterFontScaleY(const int v) { if (m_printer) m_printer->setFontScaleY(v); }
    int getPrinterFontScaleX() const override { return m_printer->getFontScaleX(); }
    int getPrinterFontScaleY() const override { return m_printer->getFontScaleY(); }
    void setPrinterText(const QString t) { if (m_printer) m_printer->setText(t); }

    // reflection
    void setReflectionEnabled(const bool on) {
        reflectionEnabled = on;
        m_printer->setReflectionEnabled(on);
        m_scroller->setReflectionEnabled(on);
        m_vuMeters->setReflectionEnabled(on);
    }

    bool getReflectionEnabled() const override { return reflectionEnabled; }

    void setReflectionOpacity(const int pct) {
        reflectionOpacity = pct / 100.0;
        m_scroller->setReflectionOpacity(reflectionOpacity);
        m_printer->setReflectionOpacity(reflectionOpacity);
        m_vuMeters->setReflectionOpacity(reflectionOpacity);
    }

    int getReflectionOpacity() const override { return static_cast<int>(reflectionOpacity * 100); }

    void setScrollerReflectionColor(const QColor c) {
        reflectionColor = c;
        m_printer->setReflectionColor(c);
    }

    QString getReflectionColor() const override { return reflectionColor.name(); }

    void stop();

    QWidget *parent;

private:
    std::unique_ptr<RasterBarsEffect> m_rasterBars;
    std::unique_ptr<StarField> m_starField;
    std::unique_ptr<RotatingObject> m_rotatingObject;
    std::unique_ptr<PrinterEffect> m_printer;
    std::unique_ptr<ScrollerEffect> m_scroller;
    std::unique_ptr<VuMetersEffect> m_vuMeters;

    bool isStopping;
    bool customScrolltextEnabled;
    bool hasInited;
    int originalWidth;
    int originalHeight;
    bool keepAspectRatio;
    qreal scaleX;
    qreal scaleY;

    void reset();

    QImage backbuf;
    int previousWidth;
    int previousHeight;
    int verticalScrollPosition;
    QString customScrolltext;
    bool reflectionEnabled;
    QColor reflectionColor;
    QPixmap m_CharacterMap;
    QPixmap m_CharacterMapPrinter;
    QColor colorVisualizerBackground;
    QString m_bitmapFont;
    QString m_bitmapFontPrinter;
    double reflectionOpacity;
};

#endif // SCROLLER_H
