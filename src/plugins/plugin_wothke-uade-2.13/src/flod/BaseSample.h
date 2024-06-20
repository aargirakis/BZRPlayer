#ifndef BASESAMPLE_H
#define BASESAMPLE_H
#include <string>
class BaseSample {
public:
	BaseSample();
	std::string name;
	int pointer;
	int length;
	int loopPtr;
	int repeat;
	int volume;
	int relative;
	int finetune;
};
#endif // BASESAMPLE_H
