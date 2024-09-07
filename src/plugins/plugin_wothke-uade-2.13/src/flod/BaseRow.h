#ifndef BASEROW_H
#define BASEROW_H
#include <string>
class BaseRow {
public:
	BaseRow();
	int step;
	int speed;
	int note;
	int sample;
	int effect;
	int effect2; //for ahx/hively pattern view
	int param;
	int param2; //for ahx/hively pattern view
	int vol; //for s3m pattern view
    std::string noteText;
};
#endif // BASEROW_H
