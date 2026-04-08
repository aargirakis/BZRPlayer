#include <QPainter>
#include "soundmanager.h"
#include "visualizerfullscreen.h"

VisualizerFullScreen::VisualizerFullScreen(Effect *effect, QWidget *parent) : QOpenGLWidget(parent) {
    this->effect = effect;
    // antialiasing, for example if we draw circles
    //    QSurfaceFormat format;
    //    format.setSamples(4);
    //    setFormat(format);
}

void VisualizerFullScreen::paintEvent(QPaintEvent *event) {
    if (const auto &sm = SoundManager::getInstance();
        isFullScreen() && sm.isPlaying()) {
        sm.getPosition(FMOD_TIMEUNIT_MODVUMETER);
        QPainter painter;
        painter.begin(this);
        //painter.setRenderHint(QPainter::Antialiasing);
        //painter.setRenderHint(QPainter::SmoothPixmapTransform);

        painter.fillRect(event->rect(), effect->getColorVisualizerBackground());
        effect->paint(&painter, event);
        painter.end();
    }
}
