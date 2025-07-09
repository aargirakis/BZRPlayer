#ifndef S2INSTRUMENT_H
#define S2INSTRUMENT_H

class S2Instrument
{
    friend class S2Player;

public:
    S2Instrument();

private:
    int wave;
    int waveLen;
    int waveDelay;
    int waveSpeed;
    int arpeggio;
    int arpeggioLen;
    int arpeggioDelay;
    int arpeggioSpeed;
    int vibrato;
    int vibratoLen;
    int vibratoDelay;
    int vibratoSpeed;
    int pitchBend;
    int pitchBendDelay;
    int attackMax;
    int attackSpeed;
    int decayMin;
    int decaySpeed;
    int sustain;
    int releaseMin;
    int releaseSpeed;
};

#endif // S2INSTRUMENT_H
