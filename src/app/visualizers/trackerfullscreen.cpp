#include "plugins.h"
#include "soundmanager.h"
#include "trackerfullscreen.h"

TrackerFullScreen::TrackerFullScreen(Tracker* tracker, QWidget* parent): QOpenGLWidget(parent)
{
    this->tracker = tracker;
}

void TrackerFullScreen::paintEvent(QPaintEvent* event)
{
    if (tracker)
    {
        const auto &sm = SoundManager::getInstance();

        if (const auto &info = sm.info; tracker->trackerView != nullptr && sm.isPlaying() &&
                                        (info->plugin == PLUGIN_libopenmpt || info->plugin == PLUGIN_libxmp ||
                                         info->plugin == PLUGIN_hivelytracker || info->plugin == PLUGIN_sunvox_lib))
        {
            QPainter painter;
            painter.begin(this);
            tracker->paint(&painter, event);
            painter.end();
        }
    }
}
