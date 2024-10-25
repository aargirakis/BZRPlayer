#ifndef JHSONG_H
#define JHSONG_H

class JHSong
{
    friend class JHPlayer;

public:
    JHSong();

private:
    int pointer;
    int speed;
    int length;
};

#endif // JHSONG_H
