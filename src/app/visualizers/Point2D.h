#ifndef POINT2D_H
#define POINT2D_H

class Point2D
{
public:
    Point2D(int x, int y, int depth, double scaleRatio)
    {
        this->x = x;
        this->y = y;
        this->depth = depth;
        this->scaleRatio = scaleRatio;
    }

    int x, y, depth;
    double scaleRatio;

protected:


private:
};

#endif
