#ifndef TRACKERFULLSCREEN_H
#define TRACKERFULLSCREEN_H

#include "visualizers/tracker.h"
#include <QMainWindow>
#include <QWidget>
#include <QOpenGLWidget>

class TrackerFullScreen : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit TrackerFullScreen(Tracker* tracker, QWidget* parent = 0);
    void paintEvent(QPaintEvent* event);

private:
    Tracker* tracker;
};

#endif // TRACKERFULLSCREEN_H
