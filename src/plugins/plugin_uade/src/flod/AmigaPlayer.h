#ifndef AMIGAPLAYER_H
#define AMIGAPLAYER_H
#include <string>
#include <vector>
#include "Amiga.h"

class BaseSample;
class BaseRow;

class AmigaPlayer
{
public:
    AmigaPlayer(Amiga* amiga);
    virtual ~AmigaPlayer();
    std::string m_title;
    std::string format;
    Amiga* amiga;

    int getVersion();

    virtual int load(void* data, unsigned int length);

    virtual std::vector<BaseSample*> getSamples();
    virtual bool getTitle(std::string& title);

    virtual void setVersion(int version);

protected:
    std::vector<BaseSample*> samples;

    int m_version;
    int m_variant;
    int m_totalSongs;
};

#endif // AMIGAPLAYER_H
