#include "scroller.h"

#include <mainwindow.h>

#include <QApplication>
#include <plugins.h>
#include <QFile>
#include <QPainterPath>
#include "soundmanager.h"

Scroller::Scroller(QWidget* parent)
{
    this->parent = parent;

    backbuf = QImage(originalWidth, originalHeight, QImage::Format_ARGB32_Premultiplied);
    hasInited = false;
    reset();

    previousHeight = 0;
    previousWidth = 0;
    scaleX = 1;
    scaleY = 1;

    m_rasterBars = std::make_unique<RasterBarsEffect>();
    m_starField = std::make_unique<StarField>();
    m_model3D = std::make_unique<RotatingModel3D>();
    m_printer = std::make_unique<PrinterEffect>();
    m_scroller = std::make_unique<ScrollerEffect>();
    m_vuMeters = std::make_unique<VuMetersEffect>();
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

    // Only reinit sub-effects on actual size changes
    if (previousHeight != originalHeight || previousWidth != originalWidth) {
        if (m_rasterBars) m_rasterBars->setCanvasSize(originalWidth, originalHeight);
        if (m_starField) m_starField->setCanvasSize(originalWidth, originalHeight);
        if (m_scroller) m_scroller->setCanvasSize(originalWidth, originalHeight);
        if (m_vuMeters) m_vuMeters->setCanvasSize(originalWidth, originalHeight);
        if (m_printer) m_printer->setCanvasSize(originalWidth, originalHeight);

        previousHeight = originalHeight;
        previousWidth  = originalWidth;
    }

    if (m_starField && m_starField->enabled())
        m_starField->paint(&buf);

    if (m_model3D && m_model3D->enabled()) {
        const double amp = SoundManager::getInstance().getSoundData(0) / 100.0;
        m_model3D->update(amp);
        m_model3D->paint(&buf, originalWidth, originalHeight);
    }

    if (m_rasterBars && m_rasterBars->enabled())
        m_rasterBars->paint(&buf);

    const int scrollerFontH = m_scroller->getFontHeight();
    const int scrollerBottomY = m_scroller->getBottomYMax();
    const int scrollerY = originalHeight / 2 - scrollerFontH / 2;

    if (reflectionEnabled) {
        buf.fillRect(
                0,
                scrollerY + m_scroller->getVerticalScrollPosition() + scrollerBottomY + scrollerFontH / 2,
                originalWidth,
                originalHeight - (scrollerY + m_scroller->getVerticalScrollPosition() + scrollerBottomY + scrollerFontH / 2),
                reflectionColor
        );
    }
    buf.setOpacity(1.0);

    const bool stereoEnabled = !(
            SoundManager::getInstance().m_Info1->plugin == PLUGIN_furnace ||
            SoundManager::getInstance().m_Info1->plugin == PLUGIN_libopenmpt ||
            SoundManager::getInstance().m_Info1->plugin == PLUGIN_libxmp ||
            SoundManager::getInstance().m_Info1->plugin == PLUGIN_hivelytracker
    );

    int maxH = reflectionEnabled
               ? (originalHeight / 2 - scrollerFontH / 2 + m_scroller->getVerticalScrollPosition() + scrollerBottomY + scrollerFontH / 2)
               : originalHeight;

    if (m_vuMeters && m_vuMeters->enabled()) {
        m_vuMeters->setAvailableHeight(maxH);
        m_vuMeters->paint(&buf, stereoEnabled);
    }

    if (m_printer && m_printer->enabled()) {
        m_printer->setVerticalScrollPosition(m_scroller->getVerticalScrollPosition());
        m_printer->setBottomYFromScroller(scrollerBottomY);
        m_printer->paint(&buf);
        buf.setOpacity(1);
        buf.fillRect(originalWidth, 0, originalWidth * 2, originalHeight, colorVisualizerBackground);
    }

    if (m_scroller && m_scroller->enabled()) {
        const bool paused = SoundManager::getInstance().GetPaused();
        m_scroller->setScale(scaleX, scaleY);
        m_scroller->paint(&buf, paused);
    }

    buf.end();

    // Blit backbuffer to screen
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


QColor Scroller::getColorVisualizerBackground() const
{
    return colorVisualizerBackground;
}

int Scroller::getResolutionWidth() const
{
    return originalWidth;
}

int Scroller::getResolutionHeight() const
{
    return originalHeight;
}

bool Scroller::getKeepAspectRatio() const
{
    return keepAspectRatio;
}

void Scroller::stop()
{
    isStopping = true;
}

QString Scroller::getCustomScrolltext() const
{
    return customScrolltext;
}

void Scroller::reset()
{
    setScrollerFont(m_bitmapFont);
    setPrinterFont(m_bitmapFontPrinter);
}

void Scroller::setKeepAspectRatio(bool enabled)
{
    keepAspectRatio = enabled;
}

void Scroller::setResolutionWidth(int width)
{
    originalWidth = width;
    if (m_rasterBars) m_rasterBars->setCanvasSize(originalWidth, originalHeight);
    reset();
}

void Scroller::setResolutionHeight(int height)
{
    originalHeight = height;
    if (m_rasterBars) m_rasterBars->setCanvasSize(originalWidth, originalHeight);
}

void Scroller::setCustomScrolltextEnabled(bool enabled)
{
    customScrolltextEnabled = enabled;
}

void Scroller::setCustomScrolltext(QString text)
{
    customScrolltext = text;
}
