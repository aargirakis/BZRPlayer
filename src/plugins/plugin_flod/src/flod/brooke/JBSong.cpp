#include "JBSong.h"

using namespace std;

JBSong::JBSong()
{
    speed = 0;
    pointer = vector<int>(4);
    length = vector<int>(4);
    restart = vector<int>(4);
}

JBSong::~JBSong()
{
    pointer.clear();
    length.clear();
    restart.clear();
}
