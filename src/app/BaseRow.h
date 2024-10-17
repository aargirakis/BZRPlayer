#ifndef BASEROW_H
#define BASEROW_H
#include <string>
class BaseRow {
public:
	BaseRow();
	int effect;
	int effect2; //for ahx/hively pattern view
	int note;
	int param;
	int param2; //for ahx/hively pattern view
	int sample;
	int speed; //remove this for flod 5 version
	int step;
	int vol; //for s3m pattern view
    std::string noteText;
};
#endif // BASEROW_H
