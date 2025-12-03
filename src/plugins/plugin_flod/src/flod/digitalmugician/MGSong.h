#ifndef MGSONG_H
#define MGSONG_H
#include <string>
#include <vector>

using namespace std;

class BaseStep;

class MGSong {
    friend class MGPlayer;

public:
    MGSong();

private:
    string title;
    int speed;
    int length;
    int loop;
    int loopStep;
    vector<BaseStep *> tracks;
};

#endif // MGSONG_H