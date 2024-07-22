#include "trackerfullscreen.h"
#include "visualizers/tracker.h"
#include <QtOpenGL>
#include "soundmanager.h"
TrackerFullScreen::TrackerFullScreen(Tracker* tracker, QWidget *parent)
    : QOpenGLWidget(parent)
{
    this->tracker = tracker;
}
void TrackerFullScreen::paintEvent(QPaintEvent *event)
{
    if(tracker)
    {

        if((tracker->m_render && SoundManager::getInstance().IsPlaying()) && (SoundManager::getInstance().m_Info1->plugin=="libopenmpt" || SoundManager::getInstance().m_Info1->plugin=="libxmp" || SoundManager::getInstance().m_Info1->plugin=="HivelyTracker" || SoundManager::getInstance().m_Info1->plugin=="SunVox"))
        {
            SoundManager::getInstance().GetPosition(FMOD_TIMEUNIT_MODVUMETER);
            QPainter painter;
            painter.begin(this);
            tracker->paint(&painter,event);
            painter.end();
        }
    }

}
