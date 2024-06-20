#include "AmigaChannel.h"
#include <iostream>
AmigaChannel::AmigaChannel(int index)
{
    panning = 1.0;
    if ((++index & 2) == 0) panning = -panning;
    level = panning;
    next = 0;
}
void AmigaChannel::setEnabled(int value)
{
    if (value != audena)
    {
        audena = value;
        audloc = pointer;
        audctr = pointer + length;
        timer  = 1.0;
        if (value) delay += 2;
    }
}
void AmigaChannel::setPeriod(int value)
{

    if (value < 60) value = 60;
      else if (value > 65535) value = 65535;
    audper = value;
}
void AmigaChannel::setVolume(int value)
{
  if (value < 0) value = 0;
  else if (value > 64) value = 64;
  audvol = value;
}
void AmigaChannel::reset()
{
      ldata = 0.0;
      rdata = 0.0;
}
int AmigaChannel::enabled()
{
    return audena;
}

void AmigaChannel::initialize()
{
    audena = 0;
    audctr = 0;
    audloc = 0;
    audper = 0;
    audvol = 0;

    timer = 0.0;
    ldata = 0.0;
    rdata = 0.0;

    delay   = 0;
    pointer = 0;
    length  = 0;
    mute = 0;
}
