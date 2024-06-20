#ifndef BASEROW_H
#define BASEROW_H
#include <string>
class BaseRow {
public:
	BaseRow();
	int step;
	int note;
	int sample;
	int effect;
	int effect2; //for ahx/hively pattern view
	int param;
	int param2; //for ahx/hively pattern view
    int vol; //for s3m pattern view
	int speed; //remove this for flod 5 version
    std::string noteText;
};
#endif // BASEROW_H
