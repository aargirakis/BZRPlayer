#include "S1Sample.h"

S1Sample::S1Sample()
{
    waveform=0;
    attackSpeed=0;
    attackMax=0;
    decaySpeed=0;
    decayMin=0;
    sustain=0;
    releaseSpeed=0;
    releaseMin=0;
    phaseShift=0;
    phaseSpeed=0;
    pitchFall=0;
    arpeggio = std::vector<int>(16);
}
