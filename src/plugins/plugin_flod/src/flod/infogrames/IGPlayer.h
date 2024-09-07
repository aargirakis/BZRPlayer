#ifndef IGPLAYER_H
#define IGPLAYER_H
#include <vector>
#include "AmigaPlayer.h"
#include "Amiga.h"
class BaseStep;
class BaseRow;
class BaseSample;
class IGVoice;
class IGBlock;
class IGPlayer : public AmigaPlayer
{
public:
    IGPlayer(Amiga* amiga);
    ~IGPlayer();
    int load(void* data, unsigned long int length, const char* filename);
private:

	std::vector<int> comData;
	std::vector<int> perData;
	std::vector<int> volData;
	std::vector<IGVoice*> voices;
    std::vector<BaseSample*> samples;
    int irqtime;
    int complete;

    int tune(IGBlock* block, std::vector<int> data, int value);

	
	
    static const int PERIODS[102];
	static const int TICKS[12];
    void process();
    void initialize();
    void printData();
    std::vector<BaseSample*> getSamples();

};

#endif // IGPLAYER_H
