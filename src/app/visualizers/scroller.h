#ifndef SCROLLER_H
#define SCROLLER_H

#include "effect.h"
#include "RasterBarsEffect.h"
#include "StarField.h"
#include "RotatingModel3D.h"
#include "PrinterEffect.h"
#include "ScrollerEffect.h"
#include "VuMetersEffect.h"

class Scroller : public Effect
{
public:
    Scroller(QWidget* parent);
    void paint(QPainter* painter, QPaintEvent* event);

    void setColorVisualizerBackground(QColor c) {colorVisualizerBackground = c;}
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

    //Rasterbars
    void setRasterBarsEnabled(bool enabled) { m_rasterBars->setEnabled(enabled); }
    bool getRasterBarsEnabled() const override { return m_rasterBars->enabled(); }
    void setNumberOfRasterBars(int amount) { m_rasterBars->setCount(amount); }
    int  getNumberOfRasterBars() const override { return m_rasterBars->count(); }
    void setRasterBarsBarHeight(int amount) { m_rasterBars->setBarHeight(amount); }
    int  getRasterBarsHeight() const override { return m_rasterBars->barHeight(); }
    void setRasterBarsVerticalSpacing(int amount) {m_rasterBars->setVerticalSpacing(amount); }
    int  getRasterBarsVerticalSpacing() const override { return m_rasterBars->verticalSpacing() ; }
    void setRasterBarsSpeed(int amount) { m_rasterBars->setSpeed(amount); }
    int  getRasterBarsSpeed()  const override { return m_rasterBars->speed(); }
    void setRasterBarsOpacity(int amount) {m_rasterBars->setOpacity(amount); }
    int  getRasterbarsOpacity() const override { return m_rasterBars->opacity(); }

    //Stars
    void setStarsEnabled(bool enabled) {m_starField->setEnabled(enabled); }
    bool getStarsEnabled() const override { return m_starField->enabled(); }
    void setNumberOfStars(int n) {m_starField->setCount(n); }
    int  getNumberOfStars() const override  { return m_starField->count(); }
    void setStarSpeed(int s) { m_starField->setSpeed(s); }
    int  getStarSpeed() const override { return m_starField->speed(); }
    void setStarsDirection(QString dir) { m_starField->setModeFromString(dir); }
    QString getStarsDirection() const override { return m_starField->modeToString();}


    //scroller
    bool getCustomScrolltextEnabled() const override {return customScrolltextEnabled;}
    void setScrollerEnabled(bool on)  {m_scroller->setEnabled(on); }
    bool getScrollerEnabled() const override { return m_scroller->enabled(); }
    void setScrollText(QString t) {m_scroller->setText(t); }
    void setScrollerFont(QString fontPng) { m_scroller->setFont(fontPng);}
    QString getFont() const override { return m_scroller->getFont(); }
    void setFontScaleX(int v) { m_scroller->setFontScaleX(v); }
    void setFontScaleY(int v) {m_scroller->setFontScaleY(v); }
    int  getFontScaleX() const override {return m_scroller->getFontScaleX();}
    int  getFontScaleY() const override {return m_scroller->getFontScaleY();}
    void setScrollSpeed(int v) {m_scroller->setScrollSpeed(v); }
    int  getScrollSpeed() const override { return m_scroller->scrollSpeed(); }
    void setAmplitude(int v)  { m_scroller->setAmplitude(v);}
    int  getAmplitude() const override {return m_scroller->amplitude();}
    void setSinusSpeed(double v) {m_scroller->setSinusSpeed(v); }
    double getSinusSpeed() const override {return m_scroller->sinusSpeed();}
    void setSinusFrequency(double v) { m_scroller->setSinusFrequency(v); }
    double getFrequency() const override { return m_scroller->sinusFrequency() ; }
    void setSinusFontScalingEnabled(bool on) {m_scroller->setSinusFontScalingEnabled(on); }
    bool getSinusFontScalingEnabled() const override { return m_scroller->sinusFontScalingEnabled(); }
    void setVerticalScrollPosition(int v) { m_scroller->setVerticalScrollPosition(v); }
    int  getVerticalScrollPosition() const override  { return m_scroller->getVerticalScrollPosition(); }

    //Rotating 3D model
    bool get3DCubeEnabled() const override { return m_model3D->enabled(); }
    void set3DCubeEnabled(bool on) {m_model3D->setEnabled(on); }
    bool get3DCubeWireframeEnabled() const override { return m_model3D->wireframeEnabled(); }
    void set3DCubeWireframeEnabled(bool on) {m_model3D->setWireframeEnabled(on); }
    int get3DCubeFocalLength() const override {return m_model3D->focalLength();}
    void set3DCubeFocalLength(int v) {m_model3D->setFocalLength(v); }
    void set3DCubeOrbitSize(int v) {m_model3D->setOrbitSize(v); }
    void set3DCubeOrbitSpeed(int v) {m_model3D->setOrbitSpeed(v); }
    int get3DCubeOrbitSize() const override {return m_model3D->orbitSize();}
    int get3DCubeOrbitSpeed() const override {return m_model3D->orbitSpeed(); }
    int get3DCubeSize() const override {return m_model3D->size();}
    void set3DCubeSize(int v) {m_model3D->setSize(v); m_model3D->initModels();}
    QString get3DCubeColorWireframe() const override { return m_model3D->wireColor().name();}
    void set3DCubeColorWireframe(QColor c) {m_model3D->setWireColor(c);}
    QString get3DCubeColor() const override{ return m_model3D->fillColor().name();}
    void set3DCubeColor(QColor c) {m_model3D->setFillColor(c);}
    bool get3DCubeOrbit() const override { return m_model3D->orbitEnabled(); }
    void set3DCubeOrbit(bool on) { m_model3D->setOrbitEnabled(on); }
    QString get3dCubeModel() const override{ return  m_model3D->model();}
    void set3dCubeModel(QString model) { m_model3D->setModel(model);}
    QString get3dCubeMaterial() const override{ return  m_model3D->material();}
    void set3dCubeMaterial(QString material) { m_model3D->setMaterial(material);}

    //VU-meters
    void setVUMeterEnabled(bool on) {if (m_vuMeters) m_vuMeters->setEnabled(on); }
    bool getVUMeterEnabled() const override { return m_vuMeters && m_vuMeters->enabled(); }
    void setVumeterWidth(double pct) { m_vuMeters->setBarWidthPercent(pct); }
    double getVumeterWidth() const override { return m_vuMeters->barWidthPercent();}
    void setVumeterOpacity(int pct) {m_vuMeters->setOpacityPercent(pct); }
    int  getVumeterOpacity() const override { return m_vuMeters->opacityPercent(); }
    void setVUMeterPeaksEnabled(bool on) {if (m_vuMeters) m_vuMeters->setPeaksEnabled(on); }
    void setHeightVisualizerPeak(int h) {if (m_vuMeters) m_vuMeters->setPeakHeight(h); }
    void setColorVisualizerTop(QColor c) { if (m_vuMeters) m_vuMeters->setTopColor(c); }
    void setColorVisualizerMiddle(QColor c) { if (m_vuMeters) m_vuMeters->setMiddleColor(c); }
    void setColorVisualizerBottom(QColor c) { if (m_vuMeters) m_vuMeters->setBottomColor(c); }
    void setColorVisualizerPeakColor(QColor c) { if (m_vuMeters) m_vuMeters->setPeakColor(c); }

    // Printer
    void setPrinterEnabled(bool on) { if (m_printer) m_printer->setEnabled(on); }
    bool getPrinterEnabled() const override { return m_printer && m_printer->enabled(); }
    void setPrinterFont(QString path) { m_printer->setFont(path); }
    QString getPrinterFont() const override { return m_printer->getFont(); }
    void setPrinterFontScaleX(int v) { if (m_printer) m_printer->setFontScaleX(v); }
    void setPrinterFontScaleY(int v) { if (m_printer) m_printer->setFontScaleY(v); }
    int  getPrinterFontScaleX() const override { return m_printer->getFontScaleX(); }
    int  getPrinterFontScaleY() const override { return m_printer->getFontScaleY(); }
    void setPrinterText(QString t) { if (m_printer) m_printer->setText(t); }

    //Reflection
    void setReflectionEnabled(bool on) { reflectionEnabled = on; m_printer->setReflectionEnabled(on);m_scroller->setReflectionEnabled(on);m_vuMeters->setReflectionEnabled(on); }
    bool getReflectionEnabled() const override { return reflectionEnabled; }
    void setReflectionOpacity(int pct) { reflectionOpacity = pct/100.0; m_scroller->setReflectionOpacity(reflectionOpacity);m_printer->setReflectionOpacity(reflectionOpacity);m_vuMeters->setReflectionOpacity(reflectionOpacity); }
    int  getReflectionOpacity() const override { return int(reflectionOpacity*100); }
    void setScrollerReflectionColor(QColor c) { reflectionColor = c; m_printer->setReflectionColor(c); }
    QString getReflectionColor() const override { return reflectionColor.name(); }

    void stop();
    QWidget* parent;

private:
    std::unique_ptr<RasterBarsEffect> m_rasterBars;
    std::unique_ptr<StarField> m_starField;
    std::unique_ptr<RotatingModel3D> m_model3D;
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
