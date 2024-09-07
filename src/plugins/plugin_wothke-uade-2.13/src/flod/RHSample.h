#ifndef RHSAMPLE_H
#define RHSAMPLE_H
#include "BaseSample.h"
#include <vector>
class RHSample : public BaseSample
{
    friend class RHPlayer;
public:
    RHSample();
private:
      int divider;
      int vibrato;
      int hiPos;
      int loPos;
      std::vector<signed char> wave;
};

#endif // RHSAMPLE_H
