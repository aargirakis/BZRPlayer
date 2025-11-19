#ifndef RHSONG_H
#define RHSONG_H
#include <vector>

using namespace std;

class RHSong
{
    friend class RHPlayer;

public:
    RHSong();
    ~RHSong();

private:
    int speed;
    vector<int> tracks;
};

#endif // RHSONG_H
