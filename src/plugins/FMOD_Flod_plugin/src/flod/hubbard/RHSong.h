#ifndef RHSONG_H
#define RHSONG_H
#include <vector>
class RHSong
{
friend class RHPlayer;
public:
    RHSong();
     ~RHSong();
private:
    int speed;
    std::vector<int> tracks;
};

#endif // RHSONG_H
