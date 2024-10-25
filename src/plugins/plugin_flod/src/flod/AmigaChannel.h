#ifndef AMIGACHANNEL_H
#define AMIGACHANNEL_H

class AmigaChannel
{
    friend class Amiga;

public:
    AmigaChannel(int index);
    void setEnabled(int value);
    void setPeriod(int value);
    void setVolume(int value);
    void reset();
    int enabled();

    AmigaChannel* next;

    double panning;
    int mute;
    int delay;
    int pointer;
    int length;


    //private:
    void initialize();
    int audena;
    int audctr;
    int audloc;
    int audper;
    int audvol;
    double timer;
    double level;
    double ldata;
    double rdata;
};

#endif // AMIGACHANNEL_H
