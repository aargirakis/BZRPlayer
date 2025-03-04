#include "IGBlock.h"

IGBlock::IGBlock()
{
    flags = 0;
    pointer = 0;
    position = 0;
    amount = 0;
    negative = 0;
    positive = 0;
    delay1 = 0;
    delay2 = 0;
}

void IGBlock::reset()
{
    flags = (flags | 1) & ~4;
    position = 0;
    negative = 0;
    positive = 0;
    delay1 = 0;
    delay2 = 0;
}
