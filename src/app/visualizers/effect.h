#ifndef EFFECT_H
#define EFFECT_H

#include <qevent.h>
#include <QPainter>

class Effect
{
public:
    Effect();

    //Settings stars
    virtual void setStarsEnabled(bool) {};
    virtual bool getStarsEnabled() const{};
    virtual void setNumberOfStars(int) {};
    virtual int getNumberOfStars() const{};
    virtual void setStarsDirection(QString){};
    virtual QString getStarsDirection() const{};
    virtual void setStarSpeed(int){};
    virtual int getStarSpeed() const{};

    //Settings visualizer
    virtual void setColorVisualizerTop(QColor){};
    virtual void setColorVisualizerBottom(QColor){};
    virtual void setColorVisualizerMiddle(QColor){};
    virtual void setColorVisualizerBackground(QColor){};
    virtual void setHeightVisualizerPeak(int){};
    virtual void setColorVisualizerPeakColor(QColor){};

    //Settings raster bars
    virtual void setRasterBarsEnabled(bool){};
    virtual bool getRasterBarsEnabled() const{};
    virtual void setNumberOfRasterBars(int){};
    virtual int getNumberOfRasterBars() const{};
    virtual void setRasterBarsSpeed(int){};
    virtual int getRasterBarsSpeed() const{};
    virtual void setRasterBarsVerticalSpacing(int){};
    virtual int getRasterBarsVerticalSpacing() const{};
    virtual void setRasterBarsBarHeight(int){};
    virtual int getRasterBarsHeight() const{};
    virtual void setRasterBarsOpacity(int){};
    virtual int getRasterbarsOpacity() const{};

    //Settings scoller
    virtual void setSinusFontScalingEnabled(bool){};
    virtual bool getSinusFontScalingEnabled() const{};
    virtual void setSinusSpeed(double){};
    virtual double getSinusSpeed() const{};
    virtual void setSinusFrequency(double){};
    virtual void setAmplitude(int){};
    virtual void setScrollSpeed(int){};
    virtual int getScrollSpeed() const{};
    virtual void setFontScaleX(int){};
    virtual int getFontScaleX() const{};
    virtual void setFontScaleY(int){};
    virtual int getFontScaleY() const{};
    virtual int getAmplitude() const{};
    virtual double getFrequency() const{};
    virtual int getVerticalScrollPosition() const{};
    virtual void setScrollerEnabled(bool){};
    virtual bool getScrollerEnabled() const{};
    virtual void setVerticalScrollPosition(int){};
    virtual void setScrollText(QString){};
    virtual void setScrollerFont(QString){};
    virtual void setCustomScrolltextEnabled(bool){};
    virtual bool getCustomScrolltextEnabled() const{};
    virtual void setCustomScrolltext(QString){};
    virtual QString getCustomScrolltext() const{};


    //Settings printer
    virtual QString getPrinterFont() const{};
    virtual void setPrinterFontScaleX(int){};
    virtual int getPrinterFontScaleX() const{};
    virtual void setPrinterFontScaleY(int){};
    virtual int getPrinterFontScaleY() const{};
    virtual void setPrinterText(QString){};
    virtual void setPrinterFont(QString){};
    virtual void setPrinterEnabled(bool){};
    virtual bool getPrinterEnabled() const{};
    virtual QString getFont() const{};

    //Settings VU-meter
    virtual void setVumeterWidth(double){};
    virtual double getVumeterWidth() const{};
    virtual void setVUMeterEnabled(bool){};
    virtual bool getVUMeterEnabled() const{};
    virtual void setVUMeterPeaksEnabled(bool){};
    virtual void setVumeterOpacity(int){};
    virtual int getVumeterOpacity() const{};

    //Settings 3d model
    virtual void set3DCubeColor(QColor){};
    virtual void set3DCubeColorWireframe(QColor){};
    virtual void set3DCubeFocalLength(int){};
    virtual void set3DCubeOrbitSize(int){};
    virtual void set3DCubeOrbitSpeed(int){};
    virtual void set3DCubeEnabled(bool){};
    virtual void set3DCubeWireframeEnabled(bool){};
    virtual void set3DCubeOrbit(bool){};
    virtual bool get3DCubeEnabled() const{}
    virtual bool get3DCubeOrbit() const{}
    virtual int get3DCubeOrbitSize() const{}
    virtual int get3DCubeOrbitSpeed() const{}
    virtual bool get3DCubeWireframeEnabled() const{}
    virtual int  get3DCubeFocalLength() const{}
    virtual QString get3DCubeColor() const{}
    virtual QString get3DCubeColorWireframe() const{}
    virtual int get3DCubeSize() const{}
    virtual void set3DCubeSize(int) {}
    virtual QString get3dCubeModel() const{}
    virtual void set3dCubeModel(QString) {}
    virtual QString get3dCubeMaterial() const{}
    virtual void set3dCubeMaterial(QString) {}

    //Settings reflection
    virtual void setScrollerReflectionColor(QColor){};
    virtual void setReflectionEnabled(bool){};
    virtual bool getReflectionEnabled() const{};
    virtual QString getReflectionColor() const{};
    virtual void setReflectionOpacity(int){};
    virtual int getReflectionOpacity() const{};

    //Settings general
    virtual void setKeepAspectRatio(bool){};
    virtual void setResolutionWidth(int){};
    virtual void setResolutionHeight(int){};
    virtual QColor getColorVisualizerBackground() const{};
    virtual int getResolutionWidth() const{};
    virtual int getResolutionHeight() const{};
    virtual bool getKeepAspectRatio() const{};



    virtual void stop(){};
    virtual void paint(QPainter* painter, QPaintEvent* event){};


};

#endif // EFFECT_H
