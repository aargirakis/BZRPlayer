#ifndef TRACKERVIEW_H
#define TRACKERVIEW_H

#include <QOpenGLWidget>
#include "visualizers/tracker.h"

class TrackerView : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit TrackerView(QWidget* parent = nullptr);
    void init();
    void paintEvent(QPaintEvent* event);
    bool inited;
    Tracker* getTracker() const;
    void setBackgroundColor(QColor newColor);

private:
    Tracker* tracker;


    bool eventFilter(QObject* obj, QEvent* event);
    QColor backgroundColor;
};

#endif // TRACKERVIEW_H
