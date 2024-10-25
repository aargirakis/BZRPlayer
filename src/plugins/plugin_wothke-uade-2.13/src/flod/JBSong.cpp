#include "JBSong.h"

JBSong::JBSong()
{
    speed = 0;
    pointer = std::vector<int>(4);
    length = std::vector<int>(4);
    restart = std::vector<int>(4);
}

JBSong::~JBSong()
{
    pointer.clear();
    length.clear();
    restart.clear();
}
