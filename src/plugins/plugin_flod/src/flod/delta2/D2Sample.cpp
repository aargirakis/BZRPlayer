#include "D2Sample.h"

using namespace std;

D2Sample::D2Sample() {
    index = 0;
    pitchBend = 0;
    synth = 0;
    table = vector<unsigned char>(48);
    vibratos = vector<unsigned char>(15);
    volumes = vector<unsigned char>(15);
}