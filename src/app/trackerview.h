#ifndef TRACKERVIEW_H
#define TRACKERVIEW_H

#include "visualizers/tracker.h"
#include <QOpenGLWidget>
#include <QObject>
#include <QWidget>

class TrackerView : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit TrackerView(QWidget* parent = 0);
    void init();
    void paintEvent(QPaintEvent* event);
    bool inited;
    Tracker* getTracker();
    void setBackgroundColor(QColor newColor);

private:
    Tracker* tracker;


    bool eventFilter(QObject* obj, QEvent* event);
    QColor backgroundColor;
};

#endif // TRACKERVIEW_H
