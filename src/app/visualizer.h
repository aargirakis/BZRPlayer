#ifndef VISUALIZER_H
#define VISUALIZER_H

#include <QOpenGLWidget>
#include "visualizers/effect.h"

class Visualizer : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit Visualizer(QWidget* parent = nullptr);
    Effect* getEffect() const;

    void setBackgroundColor(QColor newColor);
    void init();
    static void stop();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    Effect* effect;
    QList<Effect*> effects;
    int currentEffect;
    bool isStopping;
    int stoppingCounter;


    QColor backgroundColor;
};

#endif // VISUALIZER_H
