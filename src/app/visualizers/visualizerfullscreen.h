#ifndef VISUALIZERFULLSCREEN_H
#define VISUALIZERFULLSCREEN_H

#include <QOpenGLWidget>
#include "visualizers/parallax.h"

class VisualizerFullScreen : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit VisualizerFullScreen(Effect* effect, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    Effect* effect;
};

#endif // VISUALIZERFULLSCREEN_H
