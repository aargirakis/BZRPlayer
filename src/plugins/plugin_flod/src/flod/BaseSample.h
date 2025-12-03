#ifndef BASESAMPLE_H
#define BASESAMPLE_H
#include <string>

using namespace std;

class BaseSample {
public:
    BaseSample();

    string name;
    int pointer;
    int length;
    int loopPtr;
    int repeat;
    int volume;
    int relative;
    int finetune;
};
#endif // BASESAMPLE_H