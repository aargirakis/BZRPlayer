#ifndef POINT3D_H
#define POINT3D_H

class Point3D
{
public:
    Point3D(const double x, const double y, const double z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    double x, y, z;
};

#endif
