#ifndef DWSong_H
#define DWSong_H
#include <vector>

class DWSong
{
    friend class DWPlayer;

public:
    DWSong();

private:
    int speed;
    int delay;
    std::vector<int> tracks;
};

#endif // DWSong_H
