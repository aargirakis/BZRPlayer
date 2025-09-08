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
    struct OrbitXform { double tx, ty, tz; bool visible; };
    OrbitXform computeCubeOrbitTransform();
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
    void set3DCubeEnabled(bool);
    void set3DCubeFilled(bool);
    void set3DCubeOrbit(bool);
    void set3DCubeColor(QColor);
    void set3DCubeColorWireframe(QColor);
    void set3DCubeWireframeEnabled(bool);
    void setPrinterEnabled(bool);
    void setVUMeterEnabled(bool);
    void setCustomScrolltextEnabled(bool);
    void setCustomScrolltext(QString);
    void setScrollerReflectionColor(QColor);
    void set3DCubeFocalLength(int);

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
    bool getCubeEnabled();
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
    bool get3DCubeEnabled();
    bool get3DCubeFilled();
    bool get3DCubeOrbit();
    bool get3DCubeFocalLength();
    bool get3DCubeWireframeEnabled();
    int getNumberOfRasterBars();
    int getRasterBarsSpeed();
    int getRasterBarsVerticalSpacing();
    int getRasterBarsHeight();

    QString getReflectionColor();
    QString get3DCubeColor();
    QString get3DCubeColorWireframe();
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
    void transform3DPointsTo2DPoints(const std::vector<Point3D>& points,
                                     const Point3D* axisRotations,
                                     std::vector<Point2D>* TransformedPointsArray,
                                     double tx, double ty, double tz);
    std::vector<Point3D> pointsArray;
    std::vector<std::array<int,4>> facesArray;
    int* edgesArray;
    Point3D* cubeAxisRotations=nullptr;
    Point3D* rotationSpeed=nullptr;
    int cubeFocalLength = 140;
    inline static constexpr int CUBE_SIZE = 450;

    QColor cubeBaseColor = QColor(0, 200, 0); // base hue for the cube
    Point3D lightDir = Point3D(0.5, 0.8, 1.0); // fake light direction

    void setCubeBaseColor(const QColor& c){ cubeBaseColor = c; }
    void setCubeLightDir(double x,double y,double z);
    inline void init3DCommon();

    static inline void normalize(Point3D& v);
    Point3D rotateOnly(const Point3D& p, const Point3D& rot); // rotate without perspective
    void    paint3dCubeFilled(QPainter* painter);

    bool   cubeOrbitEnabled  = false;  // toggle
    double cubeOrbitAngle    = 0.0;    // radians
    double cubeOrbitSpeed    = 0.02;   // radians per frame
    double cubeOrbitRadius   = 260.0;   // X–Z circle radius
    double cubeOrbitCenterZ  = 270.0;  // center of the circle along camera Z
    double cubeIdleCenterZ = 30.0;
    double cubeIdleY       = 0.0;
    double cubeOrbitY        = 0.0;    // height of the orbit plane
    bool   cubeWireframeEnabled = true;
    QColor cubeWireColor        = Qt::red;
    int    cubeWireWidth        = 1;     // px


    //end 3d cube



// Near-plane handling: when orbit would get too close to camera
    enum class CubeNearPolicy { ClampInFront, HideWhenTooClose };
    CubeNearPolicy cubeNearPolicy = CubeNearPolicy::ClampInFront;

// Near margin in pixels (screen-space denom safety): focalLength + z >= nearMargin
    double cubeNearMargin = 12.0;

// Optional: approximate cube "radius" along Z to keep whole cube safe
    double cubeBoundRadius = CUBE_SIZE * 1.8; // ~sqrt(3)*CUBE_SIZE; tweak if needed

/// Setters
    void setCubeOrbitEnabled(bool on)                 { cubeOrbitEnabled = on; }
    void setCubeOrbitSpeed(double radPerFrame)        { cubeOrbitSpeed = radPerFrame; }
    void setCubeOrbitParams(double radius, double centerZ, double y = 0.0) {
        cubeOrbitRadius = radius; cubeOrbitCenterZ = centerZ; cubeOrbitY = y;
    }
    void setCubeNearPolicyClamp(bool clamp) {
        cubeNearPolicy = clamp ? CubeNearPolicy::ClampInFront : CubeNearPolicy::HideWhenTooClose;
    }
    void setCubeNearMargin(double px) { cubeNearMargin = px; }


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

    enum class StarsMode : uint8_t { Right, Left, Up, Down, In, Out };

    StarsMode starsMode { StarsMode::Right };

    QPen starPens[8];
    QVector<QPoint> starBuckets[8];

    void buildStarPens();
    void reinitLinearStar(int i, int W, int H);
    void reinitZoomStar(int i, bool farAway); // farAway=true => z=100, else z=1

    void paintStars(QPainter* painter);
    void paintScroller(QPainter* painter, QPaintEvent* event);
    void paintVUMeters(QPainter* painter, QPaintEvent* event, bool stereo);
    void paintRasterBars(QPainter* painter, QPaintEvent* event);
    void paint3dCube(QPainter* painter);
    void createRasterBar(QPainter* painter, int offset, int numBars);
    void printText(QPainter* painter, QPaintEvent* event);
    bool setBitmapFont(QString);
    bool setBitmapFontPrinter(QString);
    void reset();
    void initialize3dCube();
    void initializeSphere(int rings = 10, int segments = 10, double radius = CUBE_SIZE);
	void updateBottomY();

    QImage backbuf;
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
    bool cubeEnabled;
    bool cubeFilled;
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
    QColor cubeColor;
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
