#ifndef PARALLAX_H
#define PARALLAX_H

#include "qevent.h"
#include "qpixmap.h"
#include "effect.h"
class Parallax : public Effect
{
public:
    Parallax();
    void paint(QPainter *painter, QPaintEvent *event);
private:
    bool inited;
    QPixmap tile1;
    QPixmap tile2;
    QPixmap tile3;
    QPixmap tile4;
    QPixmap tile5;
    QPixmap tile6;
    QPixmap tile7;
};

#endif // PARALLAX_H
