#ifndef SCROLLER_H
#define SCROLLER_H

#include "effect.h"
#include "Point2D.h"
#include "Point3D.h"

class Scroller : public Effect
{
public:
    Scroller(QWidget* parent);
    void paint(QPainter* painter, QPaintEvent* event);

    void setVumeterWidth(double);
    void setSinusSpeed(double);
    void setSinusFrequency(double);
    void setAmplitude(int);
    void setScrollSpeed(int);
    void setFontScaleX(int);
    void setFontScaleY(int);
    void setPrinterFontScaleX(int);
    void setPrinterFontScaleY(int);
    void setVerticalScrollPosition(int);
    void setScrollText(QString);
    void setPrinterText(QString);
    void setReflectionEnabled(bool);
    bool setScrollerFont(QString);
    bool setPrinterFont(QString);
    void setStarsEnabled(bool);
    void setScrollerEnabled(bool);
    void setRasterBarsEnabled(bool);
    void setPrinterEnabled(bool);
    void setVUMeterEnabled(bool);
    void setCustomScrolltextEnabled(bool);
    void setCustomScrolltext(QString);
    void setScrollerReflectionColor(QColor);

    void setNumberOfStars(int);
    void setNumberOfRasterBars(int);
    void setRasterBarsSpeed(int);
    void setRasterBarsOpacity(int);
    void setRasterBarsVerticalSpacing(int);
    void setRasterBarsBarHeight(int);
    void setStarsDirection(QString);
    void setStarSpeed(int);

    void setColorVisualizerTop(QColor);
    void setColorVisualizerBottom(QColor);
    void setColorVisualizerMiddle(QColor);
    void setColorVisualizerPeakColor(QColor);
    void setColorVisualizerBackground(QColor);
    void setHeightVisualizerPeak(int);
    void setVUMeterPeaksEnabled(bool);

    void setReflectionOpacity(int);
    int getReflectionOpacity();
    bool getSinusFontScalingEnabled();
    void setSinusFontScalingEnabled(bool);

    void setResolutionWidth(int);
    void setResolutionHeight(int);
    void setKeepAspectRatio(bool);
    int getResolutionWidth();
    int getResolutionHeight();
    bool getKeepAspectRatio();
    qreal getScaleX();
    qreal getScaleY();

    void setVumeterOpacity(int);
    int getVumeterOpacity();
    int getRasterbarsOpacity();
    QColor getColorVisualizerBackground();


    QString replaceIllegalLetters(QString);

    QString getStarsDirection();
    int getNumberOfStars();
    int getStarSpeed();
    bool getStarsEnabled();
    bool getVUMeterEnabled();
    bool getScrollerEnabled();
    bool getCustomScrolltextEnabled();
    QString getCustomScrolltext();
    bool getPrinterEnabled();
    int getAmplitude();
    double getFrequency();
    double getSinusSpeed();
    int getScrollSpeed();
    int getVerticalScrollPosition();
    int getFontScaleX();
    int getFontScaleY();
    int getPrinterFontScaleX();
    int getPrinterFontScaleY();
    bool getReflectionEnabled();
    double getVumeterWidth();
    bool getRasterBarsEnabled();
    int getNumberOfRasterBars();
    int getRasterBarsSpeed();
    int getRasterBarsVerticalSpacing();
    int getRasterBarsHeight();

    QString getReflectionColor();
    QString getFont();
    QString getPrinterFont();
    void stop();

    QWidget* parent;

private:
    double counterRasterBar;
    double sineAngleRasterBar;
    bool isStopping;

    struct Star
    {
        double x;
        double y;
        double z;
        double coord1;
        double coord2;
        double speed;
    };

    //3d cube
    void transform3DPointsTo2DPoints(std::vector<Point3D> points, Point3D* axisRotations,
                                     std::vector<Point2D>* TransformedPointsArray);
    std::vector<Point3D> pointsArray;
    std::vector<int*> facesArray;
    int* edgesArray;
    Point3D* cubeAxisRotations;
    Point3D* rotationSpeed;
    int focalLength;
    //ed 3d cube


    Star* stars;
    bool hasInited;

    struct Peak
    {
        int currentY;
        int timeLeftStill;
        int currentSpeedDrop;
    };

    QColor colorVisualizerTop;
    QColor colorVisualizerMiddle;
    QColor colorVisualizerBottom;
    QColor colorVisualizerPeak;
    QColor colorVisualizerBackground;
    bool vumeterPeaksEnabled;
    int heightPeak;
    Peak peaks[64];

    double sineAngle;
    double x[130];
    int chars[130];
    int charsPrinter[130];
    int letters;
    int position;
    int bottomY;

    int originalWidth;
    int originalHeight;
    bool keepAspectRatio;
    qreal scaleX;
    qreal scaleY;


    void paintStars(QPainter* painter, QPaintEvent* event);
    void paintScroller(QPainter* painter, QPaintEvent* event);
    void paintVUMeters(QPainter* painter, QPaintEvent* event, bool stereo);
    void paintRasterBars(QPainter* painter, QPaintEvent* event);
    void paint3dCube(QPainter* painter, QPaintEvent* event);
    void createRasterBar(QPainter* painter, int offset, int numBars);
    void printText(QPainter* painter, QPaintEvent* event);
    bool setBitmapFont(QString);
    bool setBitmapFontPrinter(QString);
    void reset();
    void initialize3dCube();
	void updateBottomY();

    int previousWidth;
    int previousHeight;

    int fontWidth;
    int fontHeight;
    int fontScaleX;
    int fontScaleY;
    int fontWidthPrinter;
    int fontHeightPrinter;
    int fontScaleXPrinter;
    int fontScaleYPrinter;
    int fontWidthOriginalPrinter;
    int fontHeightOriginalPrinter;
    int fontWidthOriginal;
    int fontHeightOriginal;
    int verticalScrollPosition;
    QString customScrolltext;
    double m_vumeterWidth;
    bool sinusFontScalingEnabled;
    bool reflectionEnabled;
    bool starsEnabled;
    bool vuMeterEnabled;
    bool scrollerEnabled;
    bool customScrolltextEnabled;
    bool printerEnabled;
    bool rasterBarsEnabled;
    int numberOfStars;
    int numberOfRasterBars;
    int rasterBarHeight;
    int rasterBarSpeed;
    int rasterBarsVerticalSpacing;
    QString starsDirection;
    QList<QList<QColor>> rasterBarsColors;
    int starSpeed;
    QColor reflectionColor;
    QString m_scrollText;
    QString m_printerText;
    QStringList m_printerTextRows;
    QPixmap m_CharacterMap;
    QPixmap m_CharacterMapPrinter;


    int m_scrollSpeed;
    int m_Amplitude;
    QString bitmapFontCharset;
    QString bitmapFontCharsetPrinter;
    QString m_bitmapFont;
    QString m_bitmapFontPrinter;
    double sinusSpeed;
    double sinusFrequency;


    double reflectionOpacity;
    double vumeterOpacity;
    double rasterbarsOpacity;
    double fadeOpacityPrinterText;
    int fadeDirectionPrinterText;
    int fadeWait;
};

#endif // SCROLLER_H
