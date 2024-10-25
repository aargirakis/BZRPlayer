#include "AmigaPlayer.h"
#include <iostream>
#include <vector>

AmigaPlayer::AmigaPlayer(Amiga* amiga)
{
    m_version = 0;
    m_variant = 0;
    this->amiga = amiga;
    this->amiga->player = this;
}

AmigaPlayer::~AmigaPlayer()
{
    delete amiga;
}

int AmigaPlayer::load(void* data, unsigned int length)
{
    amiga->reset();
    amiga->setup();
    m_version = 0;
    m_variant = 0;
    return 0;
}

int AmigaPlayer::getVersion()
{
    return m_version;
}


std::vector<BaseSample*> AmigaPlayer::getSamples()
{
    return std::vector<BaseSample*>(0);
}

bool AmigaPlayer::getTitle(std::string& title)
{
    title = "";
    return false;
}


void AmigaPlayer::setVersion(int version)
{
}
