#include "visualizerfullscreen.h"
#include "soundmanager.h"
#include <QtOpenGL>

VisualizerFullScreen::VisualizerFullScreen(Effect* effect, QWidget* parent): QOpenGLWidget(parent)
{
    this->effect = effect;
    //Antialiasing, for example if we draw circles
    //    QSurfaceFormat format;
    //    format.setSamples(4);
    //    setFormat(format);
}

void VisualizerFullScreen::paintEvent(QPaintEvent* event)
{
    if (isFullScreen() && SoundManager::getInstance().IsPlaying())
    {
        SoundManager::getInstance().GetPosition(FMOD_TIMEUNIT_MODVUMETER);
        QPainter painter;
        painter.begin(this);
        //painter.setRenderHint(QPainter::Antialiasing);
        //painter.setRenderHint(QPainter::SmoothPixmapTransform);

        painter.fillRect(event->rect(), effect->getColorVisualizerBackground());
        effect->paint(&painter, event);
        painter.end();
    }
}
