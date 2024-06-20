#include "RHSong.h"

RHSong::RHSong()
{
    tracks = std::vector<int>();
    speed=0;
}
RHSong::~RHSong()
{
     tracks.clear();
}
