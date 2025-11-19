#ifndef JBSONG_H
#define JBSONG_H

#include <vector>

using namespace std;

class JBSong
{
    friend class JBPlayer;

public:
    JBSong();
    ~JBSong();

private:
    int speed;

    vector<int> pointer;
    vector<int> length;
    vector<int> restart;
};

#endif // JBSONG_H
