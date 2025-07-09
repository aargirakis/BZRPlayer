#include "BDVoice.h"

BDVoice::BDVoice(int index)
{
    this->index = index;
    next = 0;
}

void BDVoice::initialize()
{
    channel = 0;
    sample = 0;
    sample2 = 0;
    bank = 0;
    trackPos = 0;
    patternPtr = 0;
    patternPos = 0;
    s1byte18 = 1;
    s1byte19 = 0;
    s1byte20 = 1;
    s1byte21 = 1;
    s1byte22 = 1;
    s1byte23 = 0;
    s1byte24 = 1;
    s1byte30 = 0;
    s1byte31 = 0;
    s1byte32 = 0;
    s1byte33 = 0;
    s1byte34 = 0;
    s1byte35 = 0;
    s1byte36 = 0;
    s1byte37 = 0;
    s1long38 = 0;
    s1byte42 = 0;
    s1byte43 = 0;
    s1byte44 = 0;
    s1byte45 = 0;
    s1word46 = 0;
    s1byte48 = 0;
    s1byte49 = 0;
    s1byte50 = 0;
    s1byte51 = 0;
    s1word52 = 0;
    s1word54 = 0;
    s1byte56 = 0;
    s1byte64 = 0;
    s1word66 = 65535;
    s1word68 = 0;
    s1word70 = 0;
    state = 0;
    period = 0;
    s2word10 = 0;
    volume = 0;
    s2word14 = 0;
    s2word16 = 0;
    s2word18 = 0;
    s2word20 = 0;
    s2word22 = 0;
    s2word24 = 0;
    type = 0;
    v1word14 = 0;
    v1word16 = 0;
    v1word18 = 0;
}
