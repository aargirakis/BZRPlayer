#include "MGSong.h"
#include "BaseStep.h"

using namespace std;

MGSong::MGSong()
{
    tracks = vector<BaseStep*>();
    title = "";
    speed = 0;
    length = 0;
    loop = 0;
    loopStep = 0;
}
