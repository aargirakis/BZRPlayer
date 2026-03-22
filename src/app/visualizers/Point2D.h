#ifndef POINT2D_H
#define POINT2D_H

class Point2D
{
public:
    Point2D(const int x, const int y, const int depth, double scaleRatio)
    {
        this->x = x;
        this->y = y;
        this->depth = depth;
    }

    int x, y, depth;
};

#endif
