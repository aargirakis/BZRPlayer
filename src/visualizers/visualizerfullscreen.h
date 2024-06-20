#ifndef VISUALIZERFULLSCREEN_H
#define VISUALIZERFULLSCREEN_H

#include "src/visualizers/parallax.h"
#include <QObject>
#include <QWidget>
#include <QOpenGLWidget>


class VisualizerFullScreen : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit VisualizerFullScreen(Effect* effect, QWidget *parent = 0);
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    Effect* effect;
};

#endif // VISUALIZERFULLSCREEN_H
