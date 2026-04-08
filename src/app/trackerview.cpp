#include "plugins.h"
#include "soundmanager.h"
#include "trackerview.h"

TrackerView::TrackerView(QWidget *parent) : QOpenGLWidget(parent) {
    setUpdateBehavior(PartialUpdate);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAutoFillBackground(false);
    inited = false;
    tracker = new Tracker();
    this->installEventFilter(this);
}

void TrackerView::init() {
    inited = true;
    tracker->init();
}

Tracker *TrackerView::getTracker() const {
    return tracker;
}

void TrackerView::paintEvent(QPaintEvent *event) {
    const auto &sm = SoundManager::getInstance();
    const auto &info = sm.info;

    if (tracker == nullptr || !inited || tracker->trackerView == nullptr || !sm.isPlaying() || sm.info == nullptr) {
        QPainter painter;
        painter.begin(this);
        painter.fillRect(event->rect(), backgroundColor);
        painter.end();
        return;
    }

    if (tracker && inited && tracker->trackerView != nullptr && sm.isPlaying() &&
        (info->plugin == PLUGIN_libopenmpt || info->plugin == PLUGIN_libxmp ||
         info->plugin == PLUGIN_hivelytracker || info->plugin == PLUGIN_sunvox_lib)) {
        QPainter painter;
        painter.begin(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        QPaintEvent full(rect());
        tracker->paint(&painter, &full);
        painter.end();
    }
}

// handles changing samples
bool TrackerView::eventFilter(QObject *obj, QEvent *event) {
    if (obj != this) {
        // pass the event on to the parent class
        return this->parent()->eventFilter(obj, event);
    }

    if (const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        event->type() == QEvent::MouseButtonRelease && mouseEvent->button() & Qt::LeftButton) {
        if (const auto &trackerView = tracker->trackerView;
            trackerView != nullptr) {
            const int x = mouseEvent->pos().x() / tracker->scale;

            if (const int y = mouseEvent->pos().y() / tracker->scale;
                x >= trackerView->getbuttonNextSampleX() &&
                x <= trackerView->getbuttonNextSampleX() + trackerView->getbuttonNextSampleWidth() &&
                y >= trackerView->getbuttonNextSampleY() &&
                y <= trackerView->getbuttonNextSampleY() + trackerView->getbuttonNextSampleHeight()) {
                if (trackerView->getCurrentSample() < tracker->info->numSamples - 1) {
                    trackerView->setCurrentSample(trackerView->getCurrentSample() + 1);
                }
            } else if (x >= trackerView->getbuttonPrevSampleX() &&
                       x <= trackerView->getbuttonPrevSampleX() + trackerView->getbuttonPrevSampleWidth() &&
                       y >= trackerView->getbuttonPrevSampleY() &&
                       y <= trackerView->getbuttonPrevSampleY() + trackerView->getbuttonPrevSampleHeight() &&
                       trackerView->getCurrentSample() > 0) {
                trackerView->setCurrentSample(trackerView->getCurrentSample() - 1);
            }
        }

        return true;
    }

    return false;
}

void TrackerView::setBackgroundColor(const QColor newColor) {
    backgroundColor = newColor;
}
