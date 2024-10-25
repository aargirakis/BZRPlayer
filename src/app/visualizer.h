#ifndef VISUALIZER_H
#define VISUALIZER_H

#include "mainwindow.h"
#include <QOpenGLWidget>
#include <QWidget>

class Visualizer : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit Visualizer(QWidget* parent = 0);
    Effect* getEffect();

    void setBackgroundColor(QColor newColor);
    void init();
    void stop();

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
