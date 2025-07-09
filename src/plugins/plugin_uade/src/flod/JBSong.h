#ifndef JBSONG_H
#define JBSONG_H

#include <vector>

class JBSong
{
    friend class JBPlayer;

public:
    JBSong();
    ~JBSong();

private:
    int speed;

    std::vector<int> pointer;
    std::vector<int> length;
    std::vector<int> restart;
};

#endif // JBSONG_H
