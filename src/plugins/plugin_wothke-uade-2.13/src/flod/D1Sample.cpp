#include "D1Sample.h"

D1Sample::D1Sample()
{
    loop = 0;
    synth =0;
    attackStep=0;
    attackDelay=0;
    decayStep =0;
    decayDelay=0;
    releaseStep=0;
    releaseDelay=0;
    sustain   =0;

    pitchBend =0;
    portamento=0;

    tableDelay=0;
    vibratoWait=0;
    vibratoStep=0;
    vibratoLen;
    arpeggio = std::vector<unsigned char>(8);
    table = std::vector<signed char>(48);




}
