#include "MKVoice.h"
#include "AmigaRow.h"
#include "AmigaSample.h"

MKVoice::MKVoice(int index)
{
    this->index = index;
    channel=0;
    sample=0;
	enabled=0;
	period=0;
	effect=0;
	param=0;
	volume=0;
	portaDir=0;
	portaPeriod=0;
	portaSpeed=0;
	vibratoPos=0;
	vibratoSpeed=0;
    next = 0;
}
void MKVoice::initialize()
{
    channel=0;
    sample=0;
	enabled=0;
	period=0;
	effect=0;
	param=0;
	volume=0;
	portaDir=0;
	portaPeriod=0;
	portaSpeed=0;
	vibratoPos=0;
	vibratoSpeed=0;
}
