#include "BPVoice.h"
#include "AmigaChannel.h"

BPVoice::BPVoice(int index)
{
    this->index = index;
    channel      =  0;
    enabled      =  0;
    restart      =  0;
    note         =  0;
    period       =  0;
    sample       =  0;
    samplePtr    =  0;
    sampleLen    =  2;
    synth        =  0;
    synthPtr     =  0;
    arpeggio     =  0;
    autoArpeggio =  0;
    autoSlide    =  0;
    vibrato      =  0;
    volume       =  0;
    volumeDef    =  0;
    adsrControl  =  0;
    adsrPtr      =  0;
    adsrCtr      =  0;
    lfoControl   =  0;
    lfoPtr       =  0;
    lfoCtr       =  0;
    egControl    =  0;
    egPtr        =  0;
    egCtr        =  0;
    egValue      =  0;
    fxControl    =  0;
    fxCtr        =  0;
    modControl   =  0;
    modPtr       =  0;
    modCtr       =  0;
    next = 0;
}
void BPVoice::initialize()
{
     channel      =  0;
     enabled      =  0;
     restart      =  0;
     note         =  0;
     period       =  0;
     sample       =  0;
     samplePtr    =  0;
     sampleLen    =  2;
     synth        =  0;
     synthPtr     = -1;
     arpeggio     =  0;
     autoArpeggio =  0;
     autoSlide    =  0;
     vibrato      =  0;
     volume       =  0;
     volumeDef    =  0;
     adsrControl  =  0;
     adsrPtr      =  0;
     adsrCtr      =  0;
     lfoControl   =  0;
     lfoPtr       =  0;
     lfoCtr       =  0;
     egControl    =  0;
     egPtr        =  0;
     egCtr        =  0;
     egValue      =  0;
     fxControl    =  0;
     fxCtr        =  0;
     modControl   =  0;
     modPtr       =  0;
     modCtr       =  0;
}
