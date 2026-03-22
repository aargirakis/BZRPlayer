#ifndef EFFECT_H
#define EFFECT_H

#include <QColor>
#include <QPaintEvent>

class Effect
{
public:
    virtual ~Effect() = default;

    Effect();

    // settings stars
    virtual void setStarsEnabled(bool) = 0;
    virtual bool getStarsEnabled() const = 0;
    virtual void setNumberOfStars(int) = 0;
    virtual int getNumberOfStars() const = 0;
    virtual void setStarsDirection(QString) = 0;
    virtual QString getStarsDirection() const = 0;
    virtual void setStarSpeed(int)= 0;
    virtual int getStarSpeed() const = 0;

    // settings visualizer
    virtual void setColorVisualizerTop(QColor) = 0;
    virtual void setColorVisualizerBottom(QColor) = 0;
    virtual void setColorVisualizerMiddle(QColor) = 0;
    virtual void setColorVisualizerBackground(QColor) = 0;
    virtual void setHeightVisualizerPeak(int) = 0;
    virtual void setColorVisualizerPeakColor(QColor) = 0;

    // settings raster bars
    virtual void setRasterBarsEnabled(bool) = 0;
    virtual bool getRasterBarsEnabled() const = 0;
    virtual void setNumberOfRasterBars(int) = 0;
    virtual int getNumberOfRasterBars() const = 0;
    virtual void setRasterBarsSpeed(int) = 0;
    virtual int getRasterBarsSpeed() const = 0;
    virtual void setRasterBarsVerticalSpacing(int) = 0;
    virtual int getRasterBarsVerticalSpacing() const = 0;
    virtual void setRasterBarsBarHeight(int) = 0;
    virtual int getRasterBarsHeight() const = 0;
    virtual void setRasterBarsOpacity(int) = 0;
    virtual int getRasterbarsOpacity() const = 0;

    // settings scoller
    virtual void setSinusFontScalingEnabled(bool) = 0;
    virtual bool getSinusFontScalingEnabled() const = 0;
    virtual void setSinusSpeed(double) = 0;
    virtual double getSinusSpeed() const = 0;
    virtual void setSinusFrequency(double) = 0;
    virtual void setAmplitude(int) = 0;
    virtual void setScrollSpeed(int) = 0;
    virtual int getScrollSpeed() const = 0;
    virtual void setFontScaleX(int) = 0;
    virtual int getFontScaleX() const = 0;
    virtual void setFontScaleY(int) = 0;
    virtual int getFontScaleY() const = 0;
    virtual int getAmplitude() const = 0;
    virtual double getFrequency() const = 0;
    virtual int getVerticalScrollPosition() const = 0;
    virtual void setScrollerEnabled(bool) = 0;
    virtual bool getScrollerEnabled() const = 0;
    virtual void setVerticalScrollPosition(int) = 0;
    virtual void setScrollText(QString) = 0;
    virtual void setScrollerFont(QString) = 0;
    virtual void setCustomScrolltextEnabled(bool) = 0;
    virtual bool getCustomScrolltextEnabled() const = 0;
    virtual void setCustomScrolltext(QString) = 0;
    virtual QString getCustomScrolltext() const = 0;

    // settings printer
    virtual QString getPrinterFont() const = 0;
    virtual void setPrinterFontScaleX(int) = 0;
    virtual int getPrinterFontScaleX() const = 0;
    virtual void setPrinterFontScaleY(int) = 0;
    virtual int getPrinterFontScaleY() const = 0;
    virtual void setPrinterText(QString) = 0;
    virtual void setPrinterFont(QString) = 0;
    virtual void setPrinterEnabled(bool) = 0;
    virtual bool getPrinterEnabled() const = 0;
    virtual QString getFont() const = 0;

    // settings vu-meter
    virtual void setVuMeterWidth(double) = 0;
    virtual double getVuMeterWidth() const = 0;
    virtual void setVuMeterEnabled(bool) = 0;
    virtual bool isVuMeterEnabled() const = 0;
    virtual void setVuMeterPeaksEnabled(bool) = 0;
    virtual void setVuMeterOpacity(int) = 0;
    virtual int getVuMeterOpacity() const = 0;

    // settings rotating object
    virtual void setRotatingObjectColor(QColor) = 0;
    virtual void setRotatingObjectColorWireframe(QColor) = 0;
    virtual void setRotatingObjectFocalLength(int) = 0;
    virtual void setRotatingObjectOrbitSize(int) = 0;
    virtual void setRotatingObjectOrbitSpeed(int) = 0;
    virtual void setRotatingObjectEnabled(bool) = 0;
    virtual void setRotatingObjectWireframeEnabled(bool) = 0;
    virtual void setRotatingObjectOrbit(bool) = 0;
    virtual bool getRotatingObjectEnabled() const = 0;
    virtual bool getRotatingObjectOrbit() const = 0;
    virtual int getRotatingObjectOrbitSize() const = 0;
    virtual int getRotatingObjectOrbitSpeed() const = 0;
    virtual bool getRotatingObjectWireframeEnabled() const = 0;
    virtual int  getRotatingObjectFocalLength() const = 0;
    virtual QString getRotatingObjectColor() const = 0;
    virtual QString getRotatingObjectColorWireframe() const = 0;
    virtual int getRotatingObjectSize() const = 0;
    virtual void setRotatingObjectSize(int) = 0;
    virtual QString getRotatingObjectModel() const = 0;
    virtual void setRotatingObjectModel(QString) = 0;
    virtual QString getRotatingObjectMaterial() const = 0;
    virtual void setRotatingObjectMaterial(QString) = 0;

    // settings reflection
    virtual void setScrollerReflectionColor(QColor) = 0;
    virtual void setReflectionEnabled(bool) = 0;
    virtual bool getReflectionEnabled() const = 0;
    virtual QString getReflectionColor() const = 0;
    virtual void setReflectionOpacity(int) = 0;
    virtual int getReflectionOpacity() const = 0;

    // settings general
    virtual void setKeepAspectRatio(bool) = 0;
    virtual void setResolutionWidth(int) = 0;
    virtual void setResolutionHeight(int) = 0;
    virtual QColor getColorVisualizerBackground() const = 0;
    virtual int getResolutionWidth() const = 0;
    virtual int getResolutionHeight() const = 0;
    virtual bool getKeepAspectRatio() const = 0;

    virtual void stop() = 0;
    virtual void paint(QPainter* painter, QPaintEvent* event) = 0;
};

#endif // EFFECT_H
