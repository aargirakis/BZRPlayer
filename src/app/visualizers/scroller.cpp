#include "scroller.h"

#include <mainwindow.h>

#include "qapplication.h"
#include "qdebug.h"
#include <math.h>
#include <plugins.h>
#include <QFile>
#include <QRandomGenerator>
#include "qpainterpath.h"
#include "qregularexpression.h"
#include "soundmanager.h"

Scroller::Scroller(QWidget* parent)
{
    this->parent = parent;
    m_printerText = "A VERY NICE TUNE";
    m_scrollText =
        "BUSINESS CLASS IS BECOMING INCREASINGLY MORE LUXURIOUS, SPACIOUS AND PRIVATE. WHETHER IT'S CUSTOM-DESIGNED SEAT AND BED CUSHIONS, BESPOKE FITTINGS AND FIXTURES, OR CO-BRANDING WITH SOME OF THE BIGGEST NAMES IN LUXURY, BUSINESS REALLY IS THE NEW FIRST CLASS ABOARD MANY PLANES.";
    customScrolltext = "";

    m_vumeterWidth = 100;
    m_bitmapFont = dataPath + RESOURCES_DIR + "/visualizer/bitmapfonts/angels_font.png";
    m_bitmapFontPrinter = dataPath + RESOURCES_DIR + "/visualizer/bitmapfonts/angels_font.png";
    fontScaleXPrinter = 1;
    fontScaleYPrinter = 1;
    m_Amplitude = 32;
    sinusFrequency = 0.0028f;
    sinusSpeed = 0.1f;
    m_scrollSpeed = 7;
    sinusFontScalingEnabled = false;
    customScrolltextEnabled = false;

    originalWidth = 320;
    originalHeight = 256;
    keepAspectRatio = false;
    fontScaleX = 1;
    fontScaleY = 2;

    starsDirection = "right";
    starsEnabled = true;
    cubeEnabled = true;
    setStarsDirection(starsDirection);
    buildStarPens();
    reflectionEnabled = true;
    reflectionColor = QColor(0, 3, 46);


    reflectionOpacity = 0.04f;

    fadeOpacityPrinterText = 0;
    fadeDirectionPrinterText = 1;
    fadeWait = 100;

    rasterBarsEnabled = true;
    rasterbarsOpacity = 1;
    sineAngleRasterBar = 0;
    counterRasterBar = 0;
    numberOfRasterBars = 4;
    rasterBarsVerticalSpacing = 2;
    setRasterBarsBarHeight(64);
    rasterBarSpeed = 50;

    backbuf = QImage(originalWidth, originalHeight, QImage::Format_ARGB32_Premultiplied);
    hasInited = false;
    reset();

    for (int a = 0; a < 64; a++)
    {
        peaks[a].currentY = 0;
        peaks[a].timeLeftStill = 0;
        peaks[a].currentSpeedDrop = 0;
    }


    previousHeight = 0;
    previousWidth = 0;
    scaleX = 1;
    scaleY = 1;


    //initialize3dCube();
    initializeSphere(10,20,CUBE_SIZE);
    normalize(lightDir);
}
void Scroller::initialize3dCube()
{
    init3DCommon();

    pointsArray.clear();
    pointsArray.reserve(8);
    pointsArray.push_back(Point3D(-CUBE_SIZE, -CUBE_SIZE,  CUBE_SIZE));
    pointsArray.push_back(Point3D( CUBE_SIZE, -CUBE_SIZE,  CUBE_SIZE));
    pointsArray.push_back(Point3D(-CUBE_SIZE,  CUBE_SIZE,  CUBE_SIZE));
    pointsArray.push_back(Point3D( CUBE_SIZE,  CUBE_SIZE,  CUBE_SIZE));
    pointsArray.push_back(Point3D(-CUBE_SIZE,  CUBE_SIZE, -CUBE_SIZE));
    pointsArray.push_back(Point3D( CUBE_SIZE,  CUBE_SIZE, -CUBE_SIZE));
    pointsArray.push_back(Point3D(-CUBE_SIZE, -CUBE_SIZE, -CUBE_SIZE));
    pointsArray.push_back(Point3D( CUBE_SIZE, -CUBE_SIZE, -CUBE_SIZE));

    facesArray.clear();
    facesArray.reserve(6);
    facesArray.push_back({0, 1, 3, 2}); // front
    facesArray.push_back({2, 3, 5, 4}); // right
    facesArray.push_back({4, 5, 7, 6}); // back
    facesArray.push_back({6, 7, 1, 0}); // left
    facesArray.push_back({1, 7, 5, 3}); // top
    facesArray.push_back({6, 0, 2, 4}); // bottom
}


void Scroller::initializeSphere(int rings, int segments, double radius)
{
    init3DCommon();
    pointsArray.clear();
    facesArray.clear();

    // Avoid placing vertices exactly at the poles to keep all faces quads
    // (no degenerate quads). Use a half-step offset in latitude.
    // Total verts = (rings + 1) * segments; total faces = rings * segments.

    pointsArray.reserve((rings + 1) * segments);

    const double PI = 3.14159265358979323846;

    auto idx = [segments](int i, int j) {
        // ring i in [0..rings], segment j in [0..segments-1]
        return i * segments + (j % segments);
    };

    // Build vertices on a lat-long grid
    for (int i = 0; i <= rings; ++i) {
        double theta = ((i + 0.5) / (rings + 1)) * PI;   // (0, PI), avoids exact poles
        double sinT  = std::sin(theta);
        double cosT  = std::cos(theta);

        for (int j = 0; j < segments; ++j) {
            double phi = (double(j) / segments) * (2.0 * PI); // [0, 2PI)
            double sinP = std::sin(phi);
            double cosP = std::cos(phi);

            double x = radius * sinT * cosP;
            double y = radius * cosT;
            double z = radius * sinT * sinP;

            pointsArray.push_back(Point3D(x, y, z));
        }
    }

    // Build quad faces between rings
    facesArray.reserve(rings * segments);
    for (int i = 0; i < rings; ++i) {
        for (int j = 0; j < segments; ++j) {
            int a = idx(i,     j);
            int b = idx(i,     j + 1);
            int c = idx(i + 1, j + 1);
            int d = idx(i + 1, j);
            // Winding: a,b,c,d (outside normal)
            facesArray.push_back({ a, b, c, d });
        }
    }
}

void Scroller::paint(QPainter* painter, QPaintEvent* event)
{
    if (backbuf.isNull() || backbuf.size() != QSize(originalWidth, originalHeight)) {
        backbuf = QImage(originalWidth, originalHeight, QImage::Format_ARGB32_Premultiplied);
    }

    backbuf.fill(colorVisualizerBackground);
    QPainter buf(&backbuf);
    buf.setRenderHint(QPainter::Antialiasing, false);
    buf.setRenderHint(QPainter::TextAntialiasing, false);
    buf.setRenderHint(QPainter::SmoothPixmapTransform, false);

    scaleX = 1.0;
    scaleY = 1.0;

    if (previousHeight != originalHeight || previousWidth != originalWidth) {
        hasInited = false;
    }
    previousHeight = originalHeight;
    previousWidth  = originalWidth;

    if (starsEnabled && !hasInited)
    {
        stars = new Star[numberOfStars];
        for (int a = 0; a < numberOfStars; a++)
        {
            if (starsDirection == "in" || starsDirection == "out")
            {
                stars[a].x = rand() % 70 - 35;
                stars[a].y = rand() % 70 - 35;
            }
            else
            {
                stars[a].x = rand() % (originalWidth * 2) - originalWidth;
                stars[a].y = rand() % (originalHeight * 2) - originalHeight;
            }
            stars[a].speed = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (starSpeed)));
            //between 0 and starSpeed;
            stars[a].z = (rand() % 100 + 1);
            stars[a].coord1 = 0;
            stars[a].coord2 = 0;
        }

        hasInited = true;
    }

        if (starsEnabled) paintStars(&buf);
        if (cubeEnabled)
        {
            cubeFilled ? paint3dCubeFilled(&buf) : paint3dCube(&buf);
        }
        if (rasterBarsEnabled) paintRasterBars(&buf, event);

        int scrollerYPosition = originalHeight / 2 - fontHeight / 2;
        if (reflectionEnabled) {
            buf.fillRect(0,
                         scrollerYPosition + verticalScrollPosition + bottomY + fontHeight / 2,
                         originalWidth,
                         originalHeight - scrollerYPosition + verticalScrollPosition + bottomY + fontHeight / 2,
                         reflectionColor);
        }
        buf.setOpacity(1);

        bool stereoEnabled = !(SoundManager::getInstance().m_Info1->plugin == PLUGIN_furnace ||
                               SoundManager::getInstance().m_Info1->plugin == PLUGIN_libopenmpt ||
                               SoundManager::getInstance().m_Info1->plugin == PLUGIN_libxmp ||
                               SoundManager::getInstance().m_Info1->plugin == PLUGIN_hivelytracker);

        if (vuMeterEnabled) paintVUMeters(&buf, event, stereoEnabled);

        if (printerEnabled) {
            printText(&buf, event);
            buf.setOpacity(1);
            buf.fillRect(originalWidth, 0, originalWidth * 2, originalHeight, colorVisualizerBackground);
        }

        if (scrollerEnabled) paintScroller(&buf, event);

        buf.setOpacity(1);
        buf.fillRect(0, originalHeight, originalWidth, originalHeight * 2, colorVisualizerBackground);
        buf.end();

        painter->save();
        painter->resetTransform();
        painter->setRenderHint(QPainter::SmoothPixmapTransform, false);

        QRect vp = painter->viewport();
        QRect target;
        if (keepAspectRatio) {
            QSize dst = QSize(originalWidth, originalHeight).scaled(vp.size(), Qt::KeepAspectRatio);
            target = QRect(QPoint(0,0), dst);
            target.moveCenter(vp.center());
        } else {
            target = vp;
        }

        painter->drawImage(target, backbuf, backbuf.rect());
        painter->restore();
}

void Scroller::printText(QPainter* painter, QPaintEvent* event)
{
    fadeWait--;
    if (fadeWait <= 0)
    {
        fadeWait = 0;
        fadeOpacityPrinterText += 0.01f * fadeDirectionPrinterText;
    }

    if (fadeOpacityPrinterText > 1)
    {
        fadeOpacityPrinterText = 1;
        fadeWait = 200;
        fadeDirectionPrinterText = -1;
    }
    else if (fadeOpacityPrinterText < 0)
    {
        fadeOpacityPrinterText = 0;
        fadeWait = 100;
        fadeDirectionPrinterText = 1;
    }


    int verticalRowSpace = 2;
    int finalX = 0;
    int finalY = (originalHeight / 2) - ((m_printerTextRows.size() + 1) * (fontHeightPrinter + verticalRowSpace));
    if (finalY < fontHeightPrinter)
    {
        finalY = fontHeightPrinter;
    }


    //reflection
    int scrollerYPosition = originalHeight / 2 - fontHeightPrinter / 2;
    int reflectionY = (scrollerYPosition + verticalScrollPosition + bottomY + fontHeightPrinter / 2) + (
        fontHeightPrinter / 2) * m_printerTextRows.size();
    reflectionY = reflectionY + fontHeightPrinter;

    int pos = 0;
    QString row;
    for (int rowCount = 0; rowCount < m_printerTextRows.size(); rowCount++)
    {
        row = m_printerTextRows.at(rowCount);


        finalX = (originalWidth / 2) - ((row.count() * fontWidthPrinter) / 2);

        for (int n = 0; n < row.length(); n++)
        {
            int x = finalX + (n * fontWidthPrinter);
            int y = finalY + (rowCount * (fontHeightPrinter + verticalRowSpace));
            painter->setOpacity(fadeOpacityPrinterText);
            painter->setTransform(QTransform().scale(scaleX, scaleY));
            painter->drawPixmap(x, y, fontWidthPrinter, fontHeightPrinter, m_CharacterMapPrinter,
                                charsPrinter[pos] * fontWidthPrinter, 0, fontWidthPrinter, fontHeightPrinter);
            //reflection

            if (reflectionEnabled)
            {
                painter->setOpacity(fadeOpacityPrinterText * reflectionOpacity);
                painter->setTransform(QTransform().scale(scaleX, -1 * scaleY));
                painter->drawPixmap(
                    x, (reflectionY + (m_printerTextRows.size() - rowCount * (fontHeightPrinter + verticalRowSpace) /
                        2)) * -1, fontWidthPrinter, fontHeightPrinter / 2, m_CharacterMapPrinter,
                    charsPrinter[pos] * fontWidthPrinter, 0, fontWidthPrinter, fontHeightPrinter);
            }
            pos++;
        }
    }
}

void Scroller::paintScroller(QPainter* painter, QPaintEvent* event)
{
    if (m_scrollText.trimmed().isEmpty()) return;
    // Sine
    for (int n = 0; n < letters; n++)
    {
        // Draw each vertical column of pixel data, per character
        for (int xC = 0; xC < fontWidth; xC += fontScaleX)
        {
            int y;
            if (!sinusFontScalingEnabled)
            {
                y = (sin(((x[n] + xC) * sinusFrequency) + (sineAngle)) * m_Amplitude) * fontScaleY;
            }
            else
            {
                y = (sin(((x[n] + xC) * sinusFrequency) + (sineAngle)) * m_Amplitude);
                y = y * fontScaleY;
            }


            //int y = -abs(sin(((x[n] + xC)*sinusFrequency) + (sineAngle)) * m_Amplitude);
            if (y > bottomY)
            {
                bottomY = y;
            }

            //Only draw if it's visible
            int scrollerYPosition = originalHeight / 2 - fontHeight / 2;
            int finalY = y + scrollerYPosition + verticalScrollPosition;

            if (x[n] + (xC) > (-1 * fontWidth) && x[n] + (xC) < originalWidth && finalY < originalHeight && finalY > (-1
                * fontHeight))
            {
                painter->setOpacity(1.0f);
                painter->setTransform(QTransform().scale(scaleX, scaleY));
                painter->drawPixmap(x[n] + (xC), finalY, fontScaleX, fontHeight, m_CharacterMap,
                                    chars[n] * fontWidth + xC, 0, fontScaleX, fontHeight);
                //reflection
                if (reflectionEnabled)
                {
                    painter->setOpacity(reflectionOpacity);
                    painter->setTransform(QTransform().scale(scaleX, -1 * scaleY));
                    painter->drawPixmap(x[n] + (xC),
                                        y - scrollerYPosition - verticalScrollPosition - (bottomY * 2) - fontHeight - (
                                            fontHeight / 2), fontScaleX, fontHeight / 2, m_CharacterMap,
                                        chars[n] * fontWidth + xC, 0, fontScaleX, fontHeight);
                }
            }
        }
        painter->setOpacity(1.0f);

        // Scroll speed
        //Don't scroll if paused
        if (!SoundManager::getInstance().GetPaused())
        {
            x[n] -= m_scrollSpeed;
        }


        // Next letter
        if (x[n] < -(fontWidth))
        {
            x[n] = (letters - 1) * (fontWidth);
            int charPos;
            charPos = bitmapFontCharset.indexOf(m_scrollText.at(position));
            chars[n] = charPos;
            position++;
            if (position >= m_scrollText.length()) position = 0;
        }
    }

    // Sine speed
    sineAngle += sinusSpeed;
}

void Scroller::paint3dCube(QPainter* painter)
{
    Q_ASSERT(rotationSpeed && cubeAxisRotations);
    painter->setOpacity(1.0f);

    double amp = SoundManager::getInstance().getSoundData(0) / 100.0;

    rotationSpeed->x = 0.04 * amp;
    rotationSpeed->y = 0.05 * amp;
    rotationSpeed->z = 0.03 * amp;

    cubeAxisRotations->y -= rotationSpeed->y;
    cubeAxisRotations->x -= rotationSpeed->x;
    cubeAxisRotations->z -= rotationSpeed->z;

    // ---- Orbit translation (camera space)
    const OrbitXform xf = computeCubeOrbitTransform();
    if (!xf.visible) return; // if policy is HideWhenTooClose

    std::vector<Point2D> screenPoints;
    screenPoints.reserve(pointsArray.size());
    transform3DPointsTo2DPoints(pointsArray, cubeAxisRotations, &screenPoints,
                                xf.tx, xf.ty, xf.tz);

    auto toPt = [&](int idx) -> QPoint {
        return QPoint(originalWidth/2  + screenPoints[idx].x,
                      originalHeight/2 + screenPoints[idx].y);
    };

    painter->save();
    painter->setPen(QPen(cubeWireColor));
    painter->setBrush(Qt::NoBrush); // wireframe

    for (int i = 0; i < (int)facesArray.size(); ++i) {
        const auto& f = facesArray[i]; // f[0..3]
        QPoint poly[4] = { toPt(f[0]), toPt(f[1]), toPt(f[2]), toPt(f[3]) };
        painter->drawPolygon(poly, 4);
    }
    painter->restore();
}


void Scroller::paintRasterBars(QPainter* painter, QPaintEvent* event)
{
    counterRasterBar += 0.001 * rasterBarSpeed;
    sineAngleRasterBar += 0.004;
    for (int i = 0; i < numberOfRasterBars; i++)
    {
        createRasterBar(painter, i, numberOfRasterBars);
    }
}

void Scroller::createRasterBar(QPainter* painter, int offset, int numBars)
{
    painter->setOpacity(rasterbarsOpacity);
    int HEIGHT = originalHeight;
    int WIDTH = originalWidth;
    double amp = (HEIGHT / 2) - (rasterBarHeight / 2);
    double y = (((originalHeight / 2) - numBars) + ((sin(
                counterRasterBar + offset / double(numBars * rasterBarsVerticalSpacing * 0.05) + (sineAngleRasterBar)) *
            amp)))
        + numBars - (rasterBarHeight / 2);


    int colorNum;
    for (int i = 0; i < rasterBarHeight; i++)
    {
        colorNum = i;
        if (colorNum > (rasterBarHeight / 2) - 1)
        {
            colorNum = rasterBarHeight - 1 - i;
        }
        painter->setPen(rasterBarsColors.at(offset).at(colorNum));
        painter->drawLine(0, y + i, WIDTH, y + i);
    }
    painter->setOpacity(1);
}

void Scroller::paintVUMeters(QPainter* painter, QPaintEvent* event, bool stereo)
{
    int numChannels = SoundManager::getInstance().m_Info1->numChannels;
    if (stereo)
    {
        numChannels = SoundManager::getInstance().m_Info1->numChannelsStream;
    }
    int maxHeight;
    if (reflectionEnabled)
    {
        maxHeight = originalHeight / 2 - fontHeight / 2 + verticalScrollPosition + bottomY + fontHeight / 2;
    }
    else
    {
        maxHeight = originalHeight;
    }

    if (vumeterPeaksEnabled)
    {
        maxHeight -= heightPeak;
    }


    int VUMETERWIDTH = (originalWidth / numChannels) * (m_vumeterWidth / 100.0);
    int WIDTH = (originalWidth / numChannels);

    int HILIGHT_WIDTH = VUMETERWIDTH * 0.1;


    int LEFT_OFFSET = (WIDTH - VUMETERWIDTH) / 2;

    for (int unsigned i = 0; i < numChannels; i++)
    {
        unsigned char volume;
        if (stereo)
        {
            volume = SoundManager::getInstance().getSoundData(i); //volume is between 0-100
        }
        else
        {
            volume = static_cast<unsigned char>(SoundManager::getInstance().m_Info1->modVUMeters[i]);
            //volume is between 0-100
        }


        int vumeterCurrentHeight = (volume * maxHeight / 100);

        int vuWidth = VUMETERWIDTH;
        int xPos = (i * WIDTH) + LEFT_OFFSET;

        QRect rectL;
        QRect rectLHiLite;
        QRect rectLDark;
        QLinearGradient linearGrad;

        if (vumeterPeaksEnabled)
        {
            rectL = QRect(xPos, maxHeight - vumeterCurrentHeight + heightPeak, vuWidth, vumeterCurrentHeight);
            rectLHiLite = QRect(xPos, maxHeight - vumeterCurrentHeight + heightPeak, HILIGHT_WIDTH,
                                vumeterCurrentHeight);
            rectLDark = QRect(xPos + vuWidth - HILIGHT_WIDTH, maxHeight - vumeterCurrentHeight + heightPeak,
                              HILIGHT_WIDTH, vumeterCurrentHeight);
            linearGrad = QLinearGradient(QPointF(0, heightPeak), QPointF(0, maxHeight + heightPeak));
        }
        else
        {
            rectL = QRect(xPos, maxHeight - vumeterCurrentHeight, vuWidth, vumeterCurrentHeight);
            rectLHiLite = QRect(xPos, maxHeight - vumeterCurrentHeight, HILIGHT_WIDTH, vumeterCurrentHeight);
            rectLDark = QRect(xPos + vuWidth - HILIGHT_WIDTH, maxHeight - vumeterCurrentHeight, HILIGHT_WIDTH,
                              vumeterCurrentHeight);
            linearGrad = QLinearGradient(QPointF(0, 0), QPointF(0, maxHeight));
        }

        //main color
        linearGrad.setColorAt(0, colorVisualizerTop);
        linearGrad.setColorAt(0.5, colorVisualizerMiddle);
        linearGrad.setColorAt(1, colorVisualizerBottom);
        painter->setOpacity(vumeterOpacity);

        painter->fillRect(rectL, QBrush(linearGrad));
        painter->fillRect(rectLHiLite, QColor(255, 255, 255, 50));
        painter->fillRect(rectLDark, QColor(0, 0, 0, 80));

        //peaks

        if (vumeterPeaksEnabled)
        {
            if (vumeterCurrentHeight >= peaks[i].currentY)
            {
                peaks[i].currentY = vumeterCurrentHeight;
                peaks[i].timeLeftStill = 10;
                peaks[i].currentSpeedDrop = 1;
            }
            else
            {
                if (peaks[i].timeLeftStill <= 0)
                {
                    peaks[i].currentSpeedDrop++;
                    peaks[i].currentY -= peaks[i].currentSpeedDrop;
                }
                else
                {
                    peaks[i].timeLeftStill -= 1;
                }
            }

            QRect peak(xPos, maxHeight - peaks[i].currentY, vuWidth, heightPeak);
            QRect peakHiLite(xPos, maxHeight - peaks[i].currentY, HILIGHT_WIDTH, heightPeak);
            QRect peakDark(xPos + vuWidth - HILIGHT_WIDTH, maxHeight - peaks[i].currentY, HILIGHT_WIDTH, heightPeak);


            painter->fillRect(peak, colorVisualizerPeak);
            painter->fillRect(peakHiLite, QColor(255, 255, 255, 50));
            painter->fillRect(peakDark, QColor(0, 0, 0, 80));
        }

        if (reflectionEnabled)
        {
            painter->setOpacity(reflectionOpacity * vumeterOpacity);

            QLinearGradient linearGradReflection;


            QRect rectL;
            QRect rectLHiLite;
            QRect rectLDark;

            if (vumeterPeaksEnabled)
            {
                rectL = QRect(xPos, maxHeight + heightPeak, vuWidth, vumeterCurrentHeight / 2);
                rectLHiLite = QRect(xPos, maxHeight + heightPeak, HILIGHT_WIDTH, vumeterCurrentHeight / 2);
                rectLDark = QRect(xPos + vuWidth - HILIGHT_WIDTH, maxHeight + heightPeak, HILIGHT_WIDTH,
                                  vumeterCurrentHeight / 2);
                linearGradReflection = QLinearGradient(QPointF(0, maxHeight + heightPeak),
                                                       QPointF(0, (maxHeight * 1.5) + heightPeak));

                QRect peak(xPos, maxHeight + (heightPeak + peaks[i].currentY / 2), vuWidth, heightPeak / 2);
                QRect peakHiLite(xPos, maxHeight + (heightPeak + peaks[i].currentY / 2), HILIGHT_WIDTH, heightPeak / 2);
                QRect peakDark(xPos + vuWidth - HILIGHT_WIDTH, maxHeight + (heightPeak + peaks[i].currentY / 2),
                               HILIGHT_WIDTH, heightPeak / 2);

                painter->fillRect(peak, colorVisualizerPeak);
                painter->fillRect(peakHiLite, QColor(255, 255, 255, 50));
                painter->fillRect(peakDark, QColor(0, 0, 0, 80));
            }
            else
            {
                rectL = QRect(xPos, maxHeight, vuWidth, vumeterCurrentHeight / 2);
                rectLHiLite = QRect(xPos, maxHeight, HILIGHT_WIDTH, vumeterCurrentHeight / 2);
                rectLDark = QRect(xPos + vuWidth - HILIGHT_WIDTH, maxHeight, HILIGHT_WIDTH, vumeterCurrentHeight / 2);
                linearGradReflection = QLinearGradient(QPointF(0, maxHeight), QPointF(0, maxHeight * 1.5));
            }


            //main color
            linearGradReflection.setColorAt(0, colorVisualizerBottom);
            linearGradReflection.setColorAt(0.5, colorVisualizerMiddle);
            linearGradReflection.setColorAt(1, colorVisualizerTop);


            painter->fillRect(rectL, QBrush(linearGradReflection));
            painter->fillRect(rectLHiLite, QColor(255, 255, 255, 50));
            painter->fillRect(rectLDark, QColor(0, 0, 0, 80));
        }
    }
    painter->setOpacity(1.0);
}

void Scroller::paintStars(QPainter* painter)
{
    const int W = originalWidth;
    const int H = originalHeight;
    const int halfW = W >> 1;
    const int halfH = H >> 1;

    // clear buckets
    for (int b = 0; b < 8; ++b) starBuckets[b].clear();

    // update + bucket
    for (int i = 0; i < numberOfStars; ++i)
    {
        Star &s = stars[i];

        switch (starsMode)
        {
            case StarsMode::Right:
                s.x += s.speed;
                if (s.x > W) { reinitLinearStar(i, W, H); s.x = -float(QRandomGenerator::global()->bounded(W)); }
                break;
            case StarsMode::Left:
                s.x -= s.speed;
                if (s.x < 0) { reinitLinearStar(i, W, H); s.x = W + float(QRandomGenerator::global()->bounded(W)); }
                break;
            case StarsMode::Up:
                s.y -= s.speed;
                if (s.y < 0) { reinitLinearStar(i, W, H); s.y = H + float(QRandomGenerator::global()->bounded(H)); }
                break;
            case StarsMode::Down:
                s.y += s.speed;
                if (s.y > H) { reinitLinearStar(i, W, H); s.y = -float(QRandomGenerator::global()->bounded(H)); }
                break;

            case StarsMode::In:
            case StarsMode::Out:
            {
                // zooming: change z, then project
                const float dz = starSpeed * 0.2f;
                if (starsMode == StarsMode::In) {
                    s.z -= dz;
                    if (s.z <= 1.f) { reinitZoomStar(i, true); }
                } else { // Out
                    s.z += dz;
                    if (s.z >= 100.f) { reinitZoomStar(i, false); }
                }

                const float invZ = 1.0f / s.z; // single division, used twice
                const int sx = int(s.x * invZ * W) + halfW;
                const int sy = int(s.y * invZ * H) + halfH;

                if ((unsigned)sx < (unsigned)W && (unsigned)sy < (unsigned)H) {
                    // Farther => dimmer. z in [1..100]
                    // bucket 0 brightest, 7 dimmest; invert so near is bright:
                    int b = 7 - qBound(0, int((s.z / 100.f) * 7.999f), 7);
                    starBuckets[b].push_back(QPoint(sx, sy));
                }
                continue; // already bucketed, skip linear path below
            }
        }

        // Linear modes: visible test + bucket by speed (brighter when faster)
        if ((unsigned)s.x < (unsigned)W && (unsigned)s.y < (unsigned)H) {
            float percent = s.speed / float(starSpeed ? starSpeed : 1);
            int b = qBound(0, int(percent * 7.999f), 7);
            starBuckets[b].push_back(QPoint(int(s.x), int(s.y)));
        }
    }

    // draw all buckets in 8 batched calls
    painter->setRenderHint(QPainter::Antialiasing, false);
    for (int b = 0; b < 8; ++b) {
        if (starBuckets[b].isEmpty()) continue;
        painter->setPen(starPens[b]);
        painter->drawPoints(starBuckets[b].constData(), starBuckets[b].size());
    }
}


void Scroller::setColorVisualizerTop(QColor newColor)
{
    colorVisualizerTop = newColor;
}

void Scroller::setColorVisualizerBottom(QColor newColor)
{
    colorVisualizerBottom = newColor;
}

void Scroller::setColorVisualizerMiddle(QColor newColor)
{
    colorVisualizerMiddle = newColor;
}

void Scroller::setColorVisualizerBackground(QColor newColor)
{
    colorVisualizerBackground = newColor;
}

void Scroller::setHeightVisualizerPeak(int height)
{
    heightPeak = height;
}

void Scroller::setColorVisualizerPeakColor(QColor newColor)
{
    colorVisualizerPeak = newColor;
}

void Scroller::setVUMeterPeaksEnabled(bool enable)
{
    vumeterPeaksEnabled = enable;
}

void Scroller::setVumeterWidth(double width)
{
    m_vumeterWidth = width;
}

void Scroller::setReflectionOpacity(int opacity)
{
    reflectionOpacity = opacity / 100.0;
}

QColor Scroller::getColorVisualizerBackground()
{
    return colorVisualizerBackground;
}

int Scroller::getReflectionOpacity()
{
    return reflectionOpacity * 100;
}

bool Scroller::getSinusFontScalingEnabled()
{
    return sinusFontScalingEnabled;
}

qreal Scroller::getScaleX()
{
    return scaleX;
}

qreal Scroller::getScaleY()
{
    return scaleY;
}

int Scroller::getResolutionWidth()
{
    return originalWidth;
}

int Scroller::getResolutionHeight()
{
    return originalHeight;
}

bool Scroller::getKeepAspectRatio()
{
    return keepAspectRatio;
}

void Scroller::setVumeterOpacity(int opacity)
{
    vumeterOpacity = opacity / 100.0;
}

int Scroller::getVumeterOpacity()
{
    return vumeterOpacity * 100;
}

int Scroller::getRasterbarsOpacity()
{
    return rasterbarsOpacity * 100;
}

void Scroller::stop()
{
    isStopping = true;
}

bool Scroller::getStarsEnabled()
{
    return starsEnabled;
}

bool Scroller::getCubeEnabled()
{
    return cubeEnabled;
}


int Scroller::getNumberOfStars()
{
    return numberOfStars;
}

int Scroller::getStarSpeed()
{
    return starSpeed;
}

bool Scroller::getVUMeterEnabled()
{
    return vuMeterEnabled;
}

bool Scroller::getScrollerEnabled()
{
    return scrollerEnabled;
}

bool Scroller::getCustomScrolltextEnabled()
{
    return customScrolltextEnabled;
}

QString Scroller::getCustomScrolltext()
{
    return customScrolltext;
}

bool Scroller::getPrinterEnabled()
{
    return printerEnabled;
}

int Scroller::getAmplitude()
{
    return m_Amplitude;
}

double Scroller::getVumeterWidth()
{
    return m_vumeterWidth;
}

double Scroller::getFrequency()
{
    return sinusFrequency;
}

double Scroller::getSinusSpeed()
{
    return sinusSpeed;
}

int Scroller::getScrollSpeed()
{
    return m_scrollSpeed;
}

int Scroller::getVerticalScrollPosition()
{
    return verticalScrollPosition;
}

int Scroller::getFontScaleX()
{
    return fontScaleX;
}

int Scroller::getFontScaleY()
{
    return fontScaleY;
}

int Scroller::getPrinterFontScaleX()
{
    return fontScaleXPrinter;
}

int Scroller::getPrinterFontScaleY()
{
    return fontScaleYPrinter;
}

bool Scroller::getReflectionEnabled()
{
    return reflectionEnabled;
}

QString Scroller::getReflectionColor()
{
    return reflectionColor.name();
}

QString Scroller::get3DCubeColor()
{
    return cubeColor.name();
}
QString Scroller::get3DCubeColorWireframe()
{
    return cubeWireColor.name();
}

QString Scroller::getFont()
{
    return m_bitmapFont;
}

QString Scroller::getPrinterFont()
{
    return m_bitmapFontPrinter;
}

bool Scroller::getRasterBarsEnabled()
{
    return rasterBarsEnabled;
}
bool Scroller::get3DCubeEnabled()
{
    return cubeEnabled;
}
bool Scroller::get3DCubeFilled()
{
    return cubeFilled;
}
bool Scroller::get3DCubeOrbit()
{
    return cubeOrbitEnabled;
}
int Scroller::getNumberOfRasterBars()
{
    return numberOfRasterBars;
}

bool Scroller::get3DCubeWireframeEnabled()
{
    return cubeWireframeEnabled;
}
bool Scroller::get3DCubeFocalLength()
{
    return cubeFocalLength;
}
int Scroller::getRasterBarsSpeed()
{
    return rasterBarSpeed;
}

int Scroller::getRasterBarsVerticalSpacing()
{
    return 33 - rasterBarsVerticalSpacing;
}

int Scroller::getRasterBarsHeight()
{
    return rasterBarHeight;
}

void Scroller::setScrollText(QString text)
{
    int pixelsPerCharacter = fontWidth;
    int spacesNeeded = (originalWidth / pixelsPerCharacter) + 3;
    //Added some extra because some fontsizes will popup into the screen otherwise
    QString spaces = " ";
    spaces = spaces.repeated(spacesNeeded);


    if (QString(text.toUpper()).trimmed().isEmpty())
    {
        m_scrollText = spaces;
    }
    else
    {
        m_scrollText = spaces + QString(text.toUpper()).trimmed();
    }
    m_scrollText = replaceIllegalLetters(m_scrollText);

    letters = std::min(spacesNeeded, 130);
    for (int n = 0; n < letters; n++)
    {
        int charPos;
        charPos = bitmapFontCharset.indexOf(m_scrollText.at(n));

        chars[n] = charPos;
        x[n] = n * fontWidth;
    }
    updateBottomY();
}

//Find the approximate bottom Y of the current scroll - with it's settings -
//so we could place the reflection floor on the right place - without scroller is active
void Scroller::updateBottomY()
{
    for (int n = 0; n < letters; n++)
    {
        // Draw each vertical column of pixel data, per character
        for (int xC = 0; xC < fontWidth; xC += fontScaleX)
        {
            int y;
            if (!sinusFontScalingEnabled)
            {
                y = (sin(((x[n] + xC) * sinusFrequency) + (sineAngle)) * m_Amplitude) * fontScaleY;
            }
            else
            {
                y = (sin(((x[n] + xC) * sinusFrequency) + (sineAngle)) * m_Amplitude);
                y = y * fontScaleY;
            }

            if (y > bottomY)
            {
                bottomY = y;
            }
        }
    }
}

QString Scroller::replaceIllegalLetters(QString text)
{
    QString a1 = "ÀÁÂÃÄÅĀĂáàāãåä";
    QString a2 = "A";
    QString b1 = "ß";
    QString b2 = "B";
    QString c1 = "Çç";
    QString c2 = "C";
    QString e1 = "ËÉÈÊèéêë&";
    QString e2 = "E";
    QString f1 = "ƒ";
    QString f2 = "F";
    QString i1 = "ÍÌÎÏìíîï";
    QString i2 = "I";
    QString n1 = "Ññ";
    QString n2 = "N";
    QString o1 = "ØÓÒÔÕÖòóôðøõöø";
    QString o2 = "O";
    QString u1 = "ÛÙÚÜùúüûµ";
    QString u2 = "U";
    QString x1 = "×";
    QString x2 = "X";
    QString y1 = "Ýýÿ";
    QString y2 = "Y";
    QString question1 = "¿";
    QString question2 = "?";
    QString exclamation1 = "¡";
    QString exclamation2 = "!";
    QString hyphen1 = "¯─~·";
    QString hyphen2 = "-";
    QString eq1 = "═‗≈";
    QString eq2 = "=";
    QString quote1 = "`´‘’“”";
    QString quote2 = "\"";
    QString one1 = "¹";
    QString one2 = "1";
    QString two1 = "²";
    QString two2 = "2";
    QString three1 = "³";
    QString three2 = "3";
    QString half1 = "½";
    QString half2 = "1/2";
    QString quarter1 = "¼";
    QString quarter2 = "1/4";
    QString threequarter1 = "¾";
    QString threequarter2 = "3/4";
    QString arrow1 = "→";
    QString arrow2 = "->";


    // Performance: Check for characters
    if (text.contains(QRegularExpression("[" + QRegularExpression::escape(
        "$/:" + a1 + b1 + c1 + e1 + f1 + i1 + n1 + o1 + u1 + x1 + y1 + question1 + exclamation1 + hyphen1 + eq1 + quote1
        + one1 + two1 + three1 + arrow1 + half1 + quarter1 + threequarter1) + "]")))
    {
        text.replace(QRegularExpression("[ëêéè]"), "e");
    }
    text.replace(QRegularExpression("[" + a1 + "]"), a2);
    text.replace(QRegularExpression("[" + b1 + "]"), b2);
    text.replace(QRegularExpression("[" + c1 + "]"), c2);
    text.replace(QRegularExpression("[" + e1 + "]"), e2);
    text.replace(QRegularExpression("[" + f1 + "]"), f2);
    text.replace(QRegularExpression("[" + i1 + "]"), i2);
    text.replace(QRegularExpression("[" + n1 + "]"), n2);
    text.replace(QRegularExpression("[" + o1 + "]"), o2);
    text.replace(QRegularExpression("[" + u1 + "]"), u2);
    text.replace(QRegularExpression("[" + x1 + "]"), x2);
    text.replace(QRegularExpression("[" + y1 + "]"), y2);
    text.replace(QRegularExpression("[" + question1 + "]"), question2);
    text.replace(QRegularExpression("[" + exclamation1 + "]"), exclamation2);
    text.replace(QRegularExpression("[" + hyphen1 + "]"), hyphen2);
    text.replace(QRegularExpression("[" + eq1 + "]"), eq2);
    text.replace(QRegularExpression("[" + quote1 + "]"), quote2);
    text.replace(QRegularExpression("[" + one1 + "]"), one2);
    text.replace(QRegularExpression("[" + two1 + "]"), two2);
    text.replace(QRegularExpression("[" + three1 + "]"), three2);
    text.replace(QRegularExpression("[" + half1 + "]"), half2);
    text.replace(QRegularExpression("[" + quarter1 + "]"), quarter2);
    text.replace(QRegularExpression("[" + threequarter1 + "]"), threequarter2);
    text.replace(QRegularExpression("[" + arrow1 + "]"), arrow2);

    return text;
}

void Scroller::setPrinterText(QString text)
{
    reset();
    m_printerTextRows.clear();
    m_printerText = QString(text.toUpper()).trimmed();
    m_printerText = replaceIllegalLetters(m_printerText);
    int maxNumberOfCharactersPerRow = originalWidth / fontWidthPrinter;
    //first split to rows for every space character
    QString row = "";

    for (int i = 0; i < m_printerText.length(); i++)
    {
        row += m_printerText.at(i);
        if (m_printerText.at(i) == ' ' || m_printerText.at(i) == '-' || m_printerText.at(i) == '.' || row.length() ==
            maxNumberOfCharactersPerRow)
        {
            if (row.trimmed() != "")
            {
                m_printerTextRows.append(row);
            }
            row = "";
        }
    }
    m_printerTextRows.append(row.trimmed());
    QStringList newList;
    //merge rows that fit in one row
    //TODO will only merge max two short rows, should be ininite
    for (int i = 0; i < m_printerTextRows.count(); i++)
    {
        if (i < m_printerTextRows.count() - 1)
        {
            if (m_printerTextRows.at(i).length() + m_printerTextRows.at(i + 1).length() <= maxNumberOfCharactersPerRow)
            {
                newList.append(QString(m_printerTextRows.at(i) + m_printerTextRows.at(i + 1)).trimmed());
                i++;
            }
            else
            {
                newList.append(m_printerTextRows.at(i).trimmed());
            }
        }
        else
        {
            newList.append(m_printerTextRows.at(i).trimmed());
        }
    }
    m_printerTextRows = newList;

    reset();
}

void Scroller::setSinusSpeed(double v)
{
    sinusSpeed = v;
    bottomY = 0;
}

void Scroller::setSinusFrequency(double v)
{
    sinusFrequency = v;
    bottomY = 0;
}

void Scroller::setAmplitude(int v)
{
    m_Amplitude = v;
    bottomY = 0;
}

void Scroller::setScrollSpeed(int v)
{
    m_scrollSpeed = v;
    bottomY = 0;
}

void Scroller::reset()
{
    setScrollerFont(m_bitmapFont);
    setPrinterFont(m_bitmapFontPrinter);
    fadeOpacityPrinterText = 0;
    fadeDirectionPrinterText = 1;
    fadeWait = 100;
}

void Scroller::setFontScaleX(int v)
{
    fontScaleX = v;
    setPrinterText(m_printerText);
    reset();
}

void Scroller::setFontScaleY(int v)
{
    fontScaleY = v;
    setPrinterText(m_printerText);
    reset();
}

void Scroller::setPrinterFontScaleX(int v)
{
    fontScaleXPrinter = v;
    reset();
}

void Scroller::setPrinterFontScaleY(int v)
{
    fontScaleYPrinter = v;
    reset();
}

bool Scroller::setScrollerFont(QString font)
{
    if (m_scrollText.isEmpty()) return false;

    int extensionPos = font.lastIndexOf('.');
    if (extensionPos == -1)
    {
        return false;
    }
    QFile myFile(font.left(extensionPos) + ".inf");
    if (myFile.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&myFile);
        QString line = stream.readLine();
        fontWidthOriginal = line.toInt();
        line = stream.readLine();
        fontHeightOriginal = line.toInt();
        line = stream.readLine();
        bitmapFontCharset = line;
        m_bitmapFont = font;
        m_CharacterMap = QPixmap(m_bitmapFont);

        m_CharacterMap = m_CharacterMap.scaled(m_CharacterMap.width() * fontScaleX,
                                               m_CharacterMap.height() * fontScaleY);
        fontWidth = fontWidthOriginal * fontScaleX;
        fontHeight = fontHeightOriginal * fontScaleY;
        position = letters;
        sineAngle = 0;
        bottomY = 0;
        setScrollText(m_scrollText);

        return true;
    }
    return false;
}

bool Scroller::setPrinterFont(QString font)
{
    int extensionPos = font.lastIndexOf('.');
    if (extensionPos == -1)
    {
        return false;
    }
    QFile myFile(font.left(extensionPos) + ".inf");
    if (myFile.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&myFile);
        QString line = stream.readLine();
        fontWidthOriginalPrinter = line.toInt();
        line = stream.readLine();
        fontHeightOriginalPrinter = line.toInt();
        line = stream.readLine();
        bitmapFontCharsetPrinter = line;
        m_bitmapFontPrinter = font;
        m_CharacterMapPrinter = QPixmap(m_bitmapFontPrinter);

        m_CharacterMapPrinter = m_CharacterMapPrinter.scaled(m_CharacterMapPrinter.width() * fontScaleXPrinter,
                                                             m_CharacterMapPrinter.height() * fontScaleYPrinter);
        fontWidthPrinter = fontWidthOriginalPrinter * fontScaleXPrinter;
        fontHeightPrinter = fontHeightOriginalPrinter * fontScaleYPrinter;

        QString row;
        int pos = 0;
        for (int rowCount = 0; rowCount < m_printerTextRows.size(); ++rowCount)
        {
            const QString& row = m_printerTextRows.at(rowCount);
            for (int n = 0; n < row.length(); ++n)
            {
                if (pos >= 130) break;
                int charPos = bitmapFontCharsetPrinter.indexOf(row.at(n));
                charsPrinter[pos++] = charPos;
            }
        }
        return true;
    }
    return false;
}

void Scroller::setReflectionEnabled(bool enabled)
{
    reflectionEnabled = enabled;
}

void Scroller::setScrollerReflectionColor(QColor newColor)
{
    reflectionColor = newColor;
}

void Scroller::set3DCubeFocalLength(int value)
{
    cubeFocalLength = value;
}


void Scroller::set3DCubeColor(QColor newColor)
{
    cubeColor = newColor;
}

void Scroller::set3DCubeColorWireframe(QColor newColor)
{
    cubeWireColor=newColor;
}

void Scroller::set3DCubeWireframeEnabled(bool enabled)
{
    cubeWireframeEnabled=enabled;
}
void Scroller::setSinusFontScalingEnabled(bool enabled)
{
    sinusFontScalingEnabled = enabled;
}

void Scroller::setKeepAspectRatio(bool enabled)
{
    keepAspectRatio = enabled;
}

void Scroller::setResolutionWidth(int width)
{
    originalWidth = width;
    reset();
}

void Scroller::setResolutionHeight(int height)
{
    originalHeight = height;
}

void Scroller::setStarsEnabled(bool enabled)
{
    starsEnabled = enabled;
}

QString Scroller::getStarsDirection()
{
    return starsDirection;
}

void Scroller::setVUMeterEnabled(bool enabled)
{
    vuMeterEnabled = enabled;
}

void Scroller::setPrinterEnabled(bool enabled)
{
    printerEnabled = enabled;
}

void Scroller::setCustomScrolltextEnabled(bool enabled)
{
    customScrolltextEnabled = enabled;
}

void Scroller::setCustomScrolltext(QString text)
{
    customScrolltext = text;
}

void Scroller::setRasterBarsEnabled(bool enabled)
{
    rasterBarsEnabled = enabled;
}
void Scroller::set3DCubeEnabled(bool enabled)
{
    cubeEnabled = enabled;
}
void Scroller::set3DCubeFilled(bool enabled)
{
    cubeFilled = enabled;
}
void Scroller::set3DCubeOrbit(bool enabled)
{
    cubeOrbitEnabled = enabled;
}
void Scroller::setNumberOfStars(int amount)
{
    numberOfStars = amount;
    if (hasInited)
    {
        delete[] stars;
    }
    hasInited = false;
    buildStarPens();
}

void Scroller::setRasterBarsVerticalSpacing(int amount)
{
    rasterBarsVerticalSpacing = 33 - amount;
}

void Scroller::setRasterBarsBarHeight(int amount)
{
    rasterBarHeight = amount;
    rasterBarsColors.clear();


    for (int n = 0; n < numberOfRasterBars; n++)
    {
        double randHue = ((double)rand() / (RAND_MAX));
        double randSaturation = ((double)rand() / (RAND_MAX));
        QList<QColor> colors;
        for (int i = 0; i <= (rasterBarHeight / 2); i++)
        {
            QColor color;
            color.setHsvF(randHue, randSaturation, (double)i / (rasterBarHeight / 2));
            colors.append(color);
        }
        rasterBarsColors.append(colors);
    }
}

void Scroller::setRasterBarsSpeed(int amount)
{
    rasterBarSpeed = amount;
}

void Scroller::setRasterBarsOpacity(int amount)
{
    rasterbarsOpacity = amount / 100.0;
}

void Scroller::setNumberOfRasterBars(int amount)
{
    numberOfRasterBars = amount;
    setRasterBarsBarHeight(rasterBarHeight); //just to create new colors
}

void Scroller::setStarSpeed(int speed)
{
    starSpeed = speed;
    if (starsEnabled && hasInited) {
        for (int a = 0; a < numberOfStars; a++) {
            stars[a].speed = QRandomGenerator::global()->generateDouble() * starSpeed;
        }
    }
    buildStarPens();
}

void Scroller::setScrollerEnabled(bool enabled)
{
    scrollerEnabled = enabled;
}

void Scroller::setVerticalScrollPosition(int v)
{
    verticalScrollPosition = v;
    bottomY = 0;
}

void Scroller::transform3DPointsTo2DPoints(const std::vector<Point3D>& points,
                                           const Point3D* axisRotations,
                                           std::vector<Point2D>* TransformedPointsArray,
                                           double tx, double ty, double tz)
{
    double sx = sin(axisRotations->x), cx = cos(axisRotations->x);
    double sy = sin(axisRotations->y), cy = cos(axisRotations->y);
    double sz = sin(axisRotations->z), cz = cos(axisRotations->z);

    TransformedPointsArray->clear();
    TransformedPointsArray->reserve(points.size());

    for (int i = (int)points.size()-1; i >= 0; --i)
    {
        double x = points[i].x;
        double y = points[i].y;
        double z = points[i].z;

        // rotate
        double xy = cx * y - sx * z;
        double xz = sx * y + cx * z;
        double yz = cy * xz - sy * x;
        double yx = sy * xz + cy * x;
        double zx = cz * yx - sz * xy;
        double zy = sz * yx + cz * xy;

        // ---- NEW: translate in camera space (after rotation, before projection)
        zx += tx;
        zy += ty;
        yz += tz;

        double scaleRatio = cubeFocalLength / (cubeFocalLength + yz);
        x = zx * scaleRatio;
        y = zy * scaleRatio;
        z = yz;

        TransformedPointsArray->push_back(Point2D((int)x, (int)y, (int)-z, scaleRatio));
    }
}
void Scroller::buildStarPens()
{
    for (int i = 0; i < 8; ++i) {
        int alpha = (i + 1) * 255 / 8;            // 8 levels
        QColor c(255, 255, 255, alpha);
        starPens[i] = QPen(c);
        starPens[i].setCosmetic(true);            // 1px regardless of transform
    }
    for (int b = 0; b < 8; ++b) {
        starBuckets[b].clear();
        starBuckets[b].reserve(numberOfStars / 4 + 8); // reduce reallocs
    }
}
void Scroller::setStarsDirection(QString direction)
{
    starsDirection = direction;
    if      (direction == "right") starsMode = StarsMode::Right;
    else if (direction == "left")  starsMode = StarsMode::Left;
    else if (direction == "up")    starsMode = StarsMode::Up;
    else if (direction == "down")  starsMode = StarsMode::Down;
    else if (direction == "in")    starsMode = StarsMode::In;
    else if (direction == "out")   starsMode = StarsMode::Out;
    else                           starsMode = StarsMode::Right;
}
void Scroller::reinitLinearStar(int i, int W, int H)
{
    Star &s = stars[i];
    // keep one axis, randomize the other; reuse QRandomGenerator to avoid modulo
    if (starsMode == StarsMode::Left || starsMode == StarsMode::Right) {
        s.y = float(QRandomGenerator::global()->bounded(H));
    } else {
        s.x = float(QRandomGenerator::global()->bounded(W));
    }
    s.speed = QRandomGenerator::global()->generateDouble() * starSpeed;
}

void Scroller::reinitZoomStar(int i, bool farAway)
{
    Star &s = stars[i];
    s.x = float(QRandomGenerator::global()->bounded(70)) - 35.0f;
    s.y = float(QRandomGenerator::global()->bounded(70)) - 35.0f;
    s.speed = QRandomGenerator::global()->generateDouble() * starSpeed;
    s.z = farAway ? 100.0f : 1.0f;
}
void Scroller::setCubeLightDir(double x,double y,double z) {
    lightDir = Point3D(x,y,z);
    normalize(lightDir);
}

static inline double invSqrt(double s) { return 1.0 / std::sqrt(s); }

inline void Scroller::normalize(Point3D& v) {
    double len2 = v.x*v.x + v.y*v.y + v.z*v.z;
    if (len2 > 0.0) {
        double inv = invSqrt(len2);
        v.x *= inv; v.y *= inv; v.z *= inv;
    }
}

// Rotate a 3D point/vector by the same Euler angles you already use,
// but DO NOT apply perspective scaling or translation.
Point3D Scroller::rotateOnly(const Point3D& p, const Point3D& rot) {
    double sx = std::sin(rot.x), cx = std::cos(rot.x);
    double sy = std::sin(rot.y), cy = std::cos(rot.y);
    double sz = std::sin(rot.z), cz = std::cos(rot.z);

    // X rotation
    double xy = cx * p.y - sx * p.z;
    double xz = sx * p.y + cx * p.z;
    double rx = p.x, ry = xy, rz = xz;

    // Y rotation
    double yz = cy * rz - sy * rx;
    double yx = sy * rz + cy * rx;
    rx = yx; ry = ry; rz = yz;

    // Z rotation
    double zx = cz * rx - sz * ry;
    double zy = sz * rx + cz * ry;
    return Point3D(zx, zy, rz);
}


Scroller::OrbitXform Scroller::computeCubeOrbitTransform()
{
    OrbitXform xf{0,0,0,true};

// If orbit is OFF: keep cube centered using idle placement (independent of orbit params)
    if (!cubeOrbitEnabled) {
        double tz = cubeIdleCenterZ;
        double ty = cubeIdleY;

        // Near-plane safety so the whole cube stays in front of camera
        const double minZ = tz - cubeBoundRadius;
        const double denomMin = cubeFocalLength + minZ;
        if (denomMin < cubeNearMargin) {
            tz = (cubeNearMargin - cubeFocalLength) + cubeBoundRadius;
        }

        xf.tx = 0.0;
        xf.ty = ty;
        xf.tz = tz;
        return xf;
    }

    // --- Orbit is ON: advance and compute circle using orbit center ---
    double amp = SoundManager::getInstance().getSoundData(0) / 100.0;
    cubeOrbitAngle += cubeOrbitSpeed * amp * 2.0;

    const double tx = cubeOrbitRadius * std::cos(cubeOrbitAngle);
    const double ty = cubeOrbitY;
    double       tz = cubeOrbitRadius * std::sin(cubeOrbitAngle) + cubeOrbitCenterZ;

    const double minZ = tz - cubeBoundRadius;
    const double denomMin = cubeFocalLength + minZ;
    if (denomMin < cubeNearMargin) {
        if (cubeNearPolicy == CubeNearPolicy::HideWhenTooClose) {
            xf.visible = false;
        } else {
            tz = (cubeNearMargin - cubeFocalLength) + cubeBoundRadius;
        }
    }

    xf.tx = tx; xf.ty = ty; xf.tz = tz;
    return xf;
}

inline void Scroller::init3DCommon()
{
    if (!rotationSpeed)     rotationSpeed     = new Point3D(0.05, 0.03, 0.04);
    if (!cubeAxisRotations) cubeAxisRotations = new Point3D(0, 0, 0);
}
void Scroller::paint3dCubeFilled(QPainter* painter)
{
    // 1) Rotation driven by audio (same as wireframe)
    double amp = SoundManager::getInstance().getSoundData(0) / 100.0;
    rotationSpeed->x = 0.04 * amp;
    rotationSpeed->y = 0.05 * amp;
    rotationSpeed->z = 0.03 * amp;

    cubeAxisRotations->x -= rotationSpeed->x;
    cubeAxisRotations->y -= rotationSpeed->y;
    cubeAxisRotations->z -= rotationSpeed->z;

    // 2) Orbit translation & near-plane handling
    const OrbitXform xf = computeCubeOrbitTransform();
    if (!xf.visible) return;

    // 3) Projected points (for drawing)
    std::vector<Point2D> screenPoints;
    screenPoints.reserve(pointsArray.size());
    transform3DPointsTo2DPoints(pointsArray, cubeAxisRotations, &screenPoints,
                                xf.tx, xf.ty, xf.tz);

    // 4) Rotated camera-space points (for normals & depth)
    std::vector<Point3D> cam;  cam.reserve(pointsArray.size());
    cam.clear();
    for (const auto& p : pointsArray) {
        Point3D r = rotateOnly(p, *cubeAxisRotations);
        // add orbit translation in camera space
        r.x += xf.tx; r.y += xf.ty; r.z += xf.tz;
        cam.push_back(r);
    }

    const int cx = originalWidth  / 2;
    const int cy = originalHeight / 2;

    struct FaceItem { int idx; double depth; };
    std::vector<FaceItem> items;
    items.reserve(facesArray.size());
    for (int i = 0; i < (int)facesArray.size(); ++i) {
        const auto& f = facesArray[i];
        double zAvg = (cam[f[0]].z + cam[f[1]].z + cam[f[2]].z + cam[f[3]].z) * 0.25;
        items.push_back({ i, zAvg });
    }

    std::sort(items.begin(), items.end(),
              [](const FaceItem& a, const FaceItem& b){ return a.depth > b.depth; });

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setPen(Qt::NoPen);

    const double ambient = 0.20; // 0..1

    for (const FaceItem& it : items) {
        const auto& f = facesArray[it.idx];

        // --- 2D backface culling by screen-space winding ---
        // NOTE: Qt Y axis grows downward, so visible faces are typically "clockwise".
        QPoint p0(cx + screenPoints[f[0]].x, cy + screenPoints[f[0]].y);
        QPoint p1(cx + screenPoints[f[1]].x, cy + screenPoints[f[1]].y);
        QPoint p2(cx + screenPoints[f[2]].x, cy + screenPoints[f[2]].y);
        int abx = p1.x() - p0.x(), aby = p1.y() - p0.y();
        int acx = p2.x() - p0.x(), acy = p2.y() - p0.y();
        int crossZ = abx*acy - aby*acx;
        // Cull faces that are CCW in a Y-down system:
        if (crossZ >= 0) continue;   // <-- this is the important flip

        // --- Face normal in camera space (translation cancels out) ---
        const Point3D& A = cam[f[0]];
        const Point3D& B = cam[f[1]];
        const Point3D& C = cam[f[2]];
        Point3D u(B.x - A.x, B.y - A.y, B.z - A.z);
        Point3D v(C.x - A.x, C.y - A.y, C.z - A.z);
        Point3D n( u.y*v.z - u.z*v.y,
                   u.z*v.x - u.x*v.z,
                   u.x*v.y - u.y*v.x );
        normalize(n);

        QColor faceColor;
        if (cubeWireframeEnabled) {
            // Flat, unshaded color
            faceColor = cubeColor;
        } else {
            // Lambert + ambient
            double ndotl = n.x*lightDir.x + n.y*lightDir.y + n.z*lightDir.z;
            double shade = std::max(0.0, ndotl);
            double intensity = std::clamp(ambient + shade*(1.0 - ambient), 0.0, 1.0);

            faceColor = cubeColor;
            faceColor.setRed  (int(faceColor.red()   * intensity));
            faceColor.setGreen(int(faceColor.green() * intensity));
            faceColor.setBlue (int(faceColor.blue()  * intensity));
        }
        painter->setBrush(faceColor);

        QPoint poly[4] = {
                p0,
                p1,
                QPoint(cx + screenPoints[f[2]].x, cy + screenPoints[f[2]].y),
                QPoint(cx + screenPoints[f[3]].x, cy + screenPoints[f[3]].y),
        };
        painter->drawConvexPolygon(poly, 4);
    }

    painter->restore();  // restore from the fill pass

    if (cubeWireframeEnabled) {
        painter->save();
        painter->setBrush(Qt::NoBrush);
        QPen pen(cubeWireColor);
        pen.setWidth(cubeWireWidth);
        pen.setCosmetic(true);
        pen.setJoinStyle(Qt::MiterJoin);
        pen.setCapStyle(Qt::SquareCap);
        painter->setPen(pen);

        for (int i = 0; i < (int)facesArray.size(); ++i) {
            const auto& f = facesArray[i];
            QPoint poly[4] = {
                    QPoint(cx + screenPoints[f[0]].x, cy + screenPoints[f[0]].y),
                    QPoint(cx + screenPoints[f[1]].x, cy + screenPoints[f[1]].y),
                    QPoint(cx + screenPoints[f[2]].x, cy + screenPoints[f[2]].y),
                    QPoint(cx + screenPoints[f[3]].x, cy + screenPoints[f[3]].y),
            };
            painter->drawPolygon(poly, 4);
        }
        painter->restore();
    }

}


