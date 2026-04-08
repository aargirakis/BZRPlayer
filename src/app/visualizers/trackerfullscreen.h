#ifndef TRACKERFULLSCREEN_H
#define TRACKERFULLSCREEN_H

#include <QOpenGLWidget>
#include "visualizers/tracker.h"

class TrackerFullScreen : public QOpenGLWidget {
    Q_OBJECT

public:
    explicit TrackerFullScreen(Tracker *tracker, QWidget *parent = nullptr);

    void paintEvent(QPaintEvent *event);

private:
    Tracker *tracker;
};

#endif // TRACKERFULLSCREEN_H
