#include "STVoice.h"
#include "AmigaRow.h"
#include "AmigaSample.h"

STVoice::STVoice(int index)
{
    this->index = index;
    channel=0;
    sample=0;
	enabled=0;
	period=0;
	last=0;
	effect=0;
	param=0;
    next = 0;
}
void STVoice::initialize()
{
    channel=0;
    sample=0;
	enabled=0;
	period=0;
	last=0;
	effect=0;
	param=0;
}
