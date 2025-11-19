#include "RHSong.h"

using namespace std;

RHSong::RHSong()
{
    tracks = vector<int>();
    speed = 0;
}

RHSong::~RHSong()
{
    tracks.clear();
}
