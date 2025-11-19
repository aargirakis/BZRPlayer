#ifndef HMSAMPLE_H
#define HMSAMPLE_H
#include "AmigaSample.h"
#include <vector>

using namespace std;

class HMSample : public AmigaSample
{
    friend class HMPlayer;
public:
    HMSample();
private:
      int finetune;
      int restart;
      int waveLen;
      vector<int> waves;
      vector<int> volumes;
};

#endif // HMSAMPLE_H
