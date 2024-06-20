#ifndef HMSAMPLE_H
#define HMSAMPLE_H
#include "AmigaSample.h"
#include <vector>
class HMSample : public AmigaSample
{
    friend class HMPlayer;
public:
    HMSample();
private:
      int finetune;
      int restart;
      int waveLen;
      std::vector<int> waves;
      std::vector<int> volumes;
};

#endif // HMSAMPLE_H
