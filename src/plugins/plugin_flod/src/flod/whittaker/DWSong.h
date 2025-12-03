#ifndef DWSong_H
#define DWSong_H
#include <vector>

using namespace std;

class DWSong {
    friend class DWPlayer;

public:
    DWSong();

private:
    int speed;
    int delay;
    vector<int> tracks;
};

#endif // DWSong_H