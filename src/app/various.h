#ifndef VARIOUS_H
#define VARIOUS_H
#include <string>
#include <QString>
#include "info.h"

using namespace std;

typedef struct
{
    QString year;
    QString filename;
    QString path;
    QString title;
    QString extension;
    QString id;
    Info* info;
    unsigned int startTime;
    unsigned int startSubsong;
    signed int startSubsongPlayList;
    int subsongs;
    unsigned int length;
    bool unknownLength;
    bool seekable;
} Song;

struct Equalizer
{
    QString name;
    float eq32;
    float eq64;
    float eq125;
    float eq250;
    float eq500;
    float eq1000;
    float eq2000;
    float eq4000;
    float eq8000;
    float eq16000;
};

inline const char* const BoolToString();

QString groupDigits(int number);
unsigned int stringTimeToMs(QString strTime);
QString msToNiceStringExact(unsigned int lenms, bool displayMilliseconds);
#endif // VARIOUS_H
