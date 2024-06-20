#ifndef EFFECT_H
#define EFFECT_H


#include "qevent.h"
#include "qpainter.h"
class Effect
{
public:
    Effect();
    virtual void paint(QPainter *painter, QPaintEvent *event){};
    virtual void setKeepAspectRatio(bool){};
    virtual void setResolutionWidth(int){};
    virtual void setResolutionHeight(int){};
    virtual void setColorVisualizerTop(QColor){};
    virtual void setColorVisualizerBottom(QColor){};
    virtual void setColorVisualizerMiddle(QColor){};
    virtual void setColorVisualizerBackground(QColor){};
    virtual void setHeightVisualizerPeak(int){};
    virtual void setColorVisualizerPeakColor(QColor){};
    virtual void setScrollerReflectionColor(QColor){};
    virtual void setVUMeterPeaksEnabled(bool){};
    virtual void setSinusFontScalingEnabled(bool){};
    virtual void setSinusSpeed(double){};
    virtual void setSinusFrequency(double){};
    virtual void setVumeterWidth(double){};
    virtual void setAmplitude(int){};
    virtual void setScrollSpeed(int){};
    virtual void setFontScaleX(int){};
    virtual void setFontScaleY(int){};
    virtual void setPrinterFontScaleX(int){};
    virtual void setPrinterFontScaleY(int){};
    virtual void setVerticalScrollPosition(int){};
    virtual void setScrollText(QString){};
    virtual void setPrinterText(QString){};
    virtual void setReflectionEnabled(bool){};
    virtual bool setScrollerFont(QString){};
    virtual bool setPrinterFont(QString){};
    virtual void setStarsEnabled(bool){};
    virtual void setCustomScrolltextEnabled(bool){};
    virtual void setCustomScrolltext(QString){};
    virtual void setVUMeterEnabled(bool){};
    virtual void setScrollerEnabled(bool){};
    virtual void setPrinterEnabled(bool){};
    virtual void setRasterBarsEnabled(bool){};
    virtual void setNumberOfStars(int){};
    virtual void setNumberOfRasterBars(int){};
    virtual void setRasterBarsSpeed(int){};
    virtual void setRasterBarsVerticalSpacing(int){};
    virtual void setRasterBarsBarHeight(int){};
    virtual void setRasterBarsOpacity(int){};
    virtual void setStarsDirection(QString){};
    virtual void setStarSpeed(int){};

    virtual void stop(){};

    virtual double getVumeterWidth(){};
    virtual int getAmplitude(){};
    virtual double getFrequency(){};
    virtual double getSinusSpeed(){};
    virtual int getScrollSpeed(){};
    virtual int getVerticalScrollPosition(){};
    virtual int getFontScaleX(){};
    virtual int getFontScaleY(){};
    virtual int getPrinterFontScaleX(){};
    virtual int getPrinterFontScaleY(){};
    virtual bool getReflectionEnabled(){};
    virtual bool getCustomScrolltextEnabled(){};
    virtual QString getCustomScrolltext(){};
    virtual QString getReflectionColor(){};
    virtual bool getStarsEnabled(){};
    virtual int getNumberOfStars(){};
    virtual QString getStarsDirection(){};
    virtual int getStarSpeed(){};
    virtual bool getPrinterEnabled(){};
    virtual bool getScrollerEnabled(){};
    virtual bool getVUMeterEnabled(){};
    virtual QString getFont(){};
    virtual QString getPrinterFont(){};
    virtual QColor getColorVisualizerBackground(){};
    virtual int getResolutionWidth(){};
    virtual int getResolutionHeight(){};
    virtual bool getKeepAspectRatio(){};
    virtual qreal getScaleX(){};
    virtual qreal getScaleY(){};
    virtual bool getRasterBarsEnabled(){};
    virtual int getNumberOfRasterBars(){};
    virtual int getRasterBarsSpeed(){};
    virtual int getRasterBarsVerticalSpacing(){};
    virtual int getRasterBarsHeight(){};;

    virtual void setReflectionOpacity(int){};
    virtual int getReflectionOpacity(){};
    virtual bool getSinusFontScalingEnabled(){};

    virtual int getRasterbarsOpacity(){};
    virtual void setVumeterOpacity(int){};
    virtual int getVumeterOpacity(){};

};

#endif // EFFECT_H
