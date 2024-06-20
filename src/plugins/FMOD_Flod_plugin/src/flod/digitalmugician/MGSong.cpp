#include "MGSong.h"
#include "BaseStep.h"


MGSong::MGSong()
{
    tracks = std::vector<BaseStep*>();
    title="";
    speed=0;
    length=0;
    loop=0;
    loopStep=0;
}
