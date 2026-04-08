#include "soundmanager.h"
#include "visualizer.h"
#include "visualizers/scroller.h"

Visualizer::Visualizer(QWidget *parent) : QOpenGLWidget(parent) {
    // antialiasing, for example if we draw circles
    QSurfaceFormat format;
    format.setSamples(4);
    setFormat(format);
    currentEffect = 0;
    stoppingCounter = 0;
    isStopping = false;
    effects.append(new Scroller(parent));
}

void Visualizer::init() {
    isStopping = false;
}

void Visualizer::stop() {
    //    isStopping=true;
    //    stoppingCounter = 0;
    //    effects.at(currentEffect)->stop();
}

void Visualizer::paintEvent(QPaintEvent *event) {
    if (const auto &sm = SoundManager::getInstance();
        sm.isPlaying()) {
        sm.getPosition(FMOD_TIMEUNIT_MODVUMETER);
        QPainter painter;
        painter.begin(this);

        const QRect rect(0, 0, width(), height());
        painter.fillRect(rect, effects.at(currentEffect)->getColorVisualizerBackground());

        effects.at(currentEffect)->paint(&painter, event);
        painter.end();
    } else if (isStopping) {
        update();

        QPainter painter;
        painter.begin(this);
        painter.setOpacity(static_cast<float>(stoppingCounter / 300.0));
        //painter.setRenderHint(QPainter::Antialiasing);
        //painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.fillRect(event->rect(), backgroundColor);

        painter.end();

        if (stoppingCounter == 300) {
            isStopping = false;
        }

        stoppingCounter++;
    } else {
        QPainter painter;
        painter.begin(this);
        painter.fillRect(event->rect(), backgroundColor);
        painter.end();
    }
}

Effect *Visualizer::getEffect() const {
    return effects.at(currentEffect);
}

void Visualizer::setBackgroundColor(const QColor newColor) {
    backgroundColor = newColor;
}
