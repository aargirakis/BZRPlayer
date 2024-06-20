#ifndef FILELOADER_H
#define FILELOADER_H

#include "Amiga.h"
#include "AmigaPlayer.h"
class FileLoader
{
public:
    FileLoader();
    AmigaPlayer* load(void* data, unsigned long int length, const char* filename);
    void setForcePlayer(int);
private:

    unsigned int force;
};

#endif // FILELOADER_H
