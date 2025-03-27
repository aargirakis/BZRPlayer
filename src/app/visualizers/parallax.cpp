#include "parallax.h"
#include "qapplication.h"

Parallax::Parallax()
{
    inited = false;
}

void Parallax::paint(QPainter* painter, QPaintEvent* event)
{
    if (!inited)
    {
        tile1 = QPixmap(QApplication::applicationDirPath() + QString::fromUtf8("/Resources/visualizer/parallax1.png"));
        tile2 = QPixmap(QApplication::applicationDirPath() + QString::fromUtf8("/Resources/visualizer/parallax2.png"));
        tile3 = QPixmap(QApplication::applicationDirPath() + QString::fromUtf8("/Resources/visualizer/parallax3.png"));
        tile4 = QPixmap(QApplication::applicationDirPath() + QString::fromUtf8("/Resources/visualizer/parallax4.png"));
        tile5 = QPixmap(QApplication::applicationDirPath() + QString::fromUtf8("/Resources/visualizer/parallax5.png"));
        tile6 = QPixmap(QApplication::applicationDirPath() + QString::fromUtf8("/Resources/visualizer/parallax6.png"));
        tile7 = QPixmap(QApplication::applicationDirPath() + QString::fromUtf8("/Resources/visualizer/parallax7.png"));

        inited = true;
    }

    qreal width = event->rect().width();
    qreal height = event->rect().height();

    painter->scale(width / (qreal)320, height / (qreal)106);

    float amp = 0.5;
    static float i7 = 0;
    QRect r7(0, 106 - 106, 320, 106);
    i7 = i7 + 1 * amp;
    painter->drawTiledPixmap(r7, tile7, QPointF(i7, 0));

    static float i6 = 0;
    QRect r6(0, 106 - 29, 320, 29);
    i6 = i6 + 2 * amp;
    painter->drawTiledPixmap(r6, tile6, QPointF(i6, 0));

    static float i5 = 0;
    QRect r5(0, 106 - 27, 320, 27);
    i5 = i5 + 4 * amp;
    painter->drawTiledPixmap(r5, tile5, QPointF(i5, 0));

    static float i4 = 0;
    QRect r4(0, 106 - 24, 320, 24);
    i4 = i4 + 6 * amp;
    painter->drawTiledPixmap(r4, tile4, QPointF(i4, 0));

    static float i3 = 0;
    QRect r3(0, 106 - 17, 320, 17);
    i3 = i3 + 8 * amp;
    painter->drawTiledPixmap(r3, tile3, QPointF(i3, 0));

    static float i2 = 0;
    QRect r2(0, 106 - 11, 320, 11);
    i2 = i2 + 10 * amp;
    painter->drawTiledPixmap(r2, tile2, QPointF(i2, 0));

    static float i1 = 0;
    QRect r1(0, 106 - 21, 320, 21);
    i1 = i1 + 14 * amp;
    painter->drawTiledPixmap(r1, tile1, QPointF(i1, 0));
    painter->scale(3, 3);
}
