#include "D2Sample.h"

D2Sample::D2Sample()
{
    index=0;
    pitchBend=0;
    synth=0;
    table = std::vector<unsigned char>(48);
    vibratos = std::vector<unsigned char>(15);
    volumes = std::vector<unsigned char>(15);

}
