#ifndef MGSONG_H
#define MGSONG_H
#include <string>
#include <vector>

class BaseStep;

class MGSong
{
    friend class MGPlayer;

public:
    MGSong();

private:
    std::string title;
    int speed;
    int length;
    int loop;
    int loopStep;
    std::vector<BaseStep*> tracks;
};

#endif // MGSONG_H
