/*==============================================================================
Load Banks Example
Copyright (c), Firelight Technologies Pty, Ltd 2012-2022.

This example demonstrates loading banks via file, memory, and user callbacks.

The banks that are loaded are:

* SFX.bank (file)
* Music.bank (memory)
* Vehicles.bank (memory-point)
* VO.bank (custom)

The loading and unloading is asynchronous, and we displays the current
state of each bank as loading is occuring.

### See Also ###
* Studio::System::loadBankFile
* Studio::System::loadBankMemory
* Studio::System::loadBankCustom
* Studio::Bank::loadSampleData
* Studio::Bank::getLoadingState
* Studio::Bank::getSampleLoadingState
* Studio::Bank::getUserData
* Studio::Bank::setUserData
==============================================================================*/
#include "fmod_studio.hpp"
#include "fmod.hpp"
#include "common.h"
#include <stdio.h>

//
// Some platforms don't support cross platform fopen.  Or you can disable this
// to see how the sample deals with bank load failures.
//
#ifdef COMMON_PLATFORM_SUPPORTS_FOPEN
    #define ENABLE_FILE_OPEN
#endif

//
// Load method as enum for our sample code
//
enum LoadBankMethod
{
    LoadBank_File,
    LoadBank_Memory,
    LoadBank_MemoryPoint,
    LoadBank_Custom
};
static const char* BANK_LOAD_METHOD_NAMES[] =
{
    "File",
    "Memory",
    "Memory-Point",
    "Custom"
};

//
// Sanity check for loading files
//
#ifdef ENABLE_FILE_OPEN
    static const size_t MAX_FILE_LENGTH = 2*1024*1024*1024ULL;
#endif

//
// Custom callbacks that just wrap fopen
//
FMOD_RESULT F_CALLBACK customFileOpen(const char *name, unsigned int *filesize, void **handle, void *userdata)
{
#ifdef ENABLE_FILE_OPEN
    // We pass the filename into our callbacks via userdata in the custom info struct
    const char* filename = (const char*)userdata;
    FILE* file = fopen(filename, "rb");
    if (!file)
    {
        return FMOD_ERR_FILE_NOTFOUND;
    }
    fseek(file, 0, SEEK_END);
    size_t length = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (length >= MAX_FILE_LENGTH)
    {
        fclose(file);
        return FMOD_ERR_FILE_BAD;
    }
    *filesize = (unsigned int)length;
    *handle = file;
    return FMOD_OK;
#else
    return FMOD_ERR_FILE_NOTFOUND;
#endif
}

FMOD_RESULT F_CALLBACK customFileClose(void *handle, void *userdata)
{
#ifdef ENABLE_FILE_OPEN
    FILE* file = (FILE*)handle;
    fclose(file);
#endif
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK customFileRead(void *handle, void *buffer, unsigned int sizebytes, unsigned int *bytesread, void *userdata)
{
    *bytesread = 0;
#ifdef ENABLE_FILE_OPEN
    FILE* file = (FILE*)handle;
    size_t read = fread(buffer, 1, sizebytes, file);
    *bytesread = (unsigned int)read;
    // If the request is larger than the bytes left in the file, then we must return EOF
    if (read < sizebytes)
    {
        return FMOD_ERR_FILE_EOF;
    }
#endif
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK customFileSeek(void *handle, unsigned int pos, void *userdata)
{
#ifdef ENABLE_FILE_OPEN
    FILE* file = (FILE*)handle;
    fseek(file, pos, SEEK_SET);
#endif
    return FMOD_OK;
}

//
// Helper function that loads a file into aligned memory buffer
//
FMOD_RESULT loadFile(const char* filename, char** memoryBase, char** memoryPtr, int* memoryLength)
{
    // If we don't support fopen then just return a single invalid byte for our file
    size_t length = 1; 

#ifdef ENABLE_FILE_OPEN
    FILE* file = fopen(filename, "rb");
    if (!file)
    {
        return FMOD_ERR_FILE_NOTFOUND;
    }
    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (length >= MAX_FILE_LENGTH)
    {
        fclose(file);
        return FMOD_ERR_FILE_BAD;
    }
#endif

    // Load into a pointer aligned to FMOD_STUDIO_LOAD_MEMORY_ALIGNMENT
    char* membase = reinterpret_cast<char*>(malloc(length + FMOD_STUDIO_LOAD_MEMORY_ALIGNMENT));
    char* memptr = (char*)(((size_t)membase + (FMOD_STUDIO_LOAD_MEMORY_ALIGNMENT-1)) & ~(FMOD_STUDIO_LOAD_MEMORY_ALIGNMENT-1));

#ifdef ENABLE_FILE_OPEN
    size_t bytesRead = fread(memptr, 1, length, file);
    fclose(file);
    if (bytesRead != length)
    {
        free(membase);
        return FMOD_ERR_FILE_BAD;
    }
#endif

    *memoryBase = membase;
    *memoryPtr = memptr;
    *memoryLength = (int)length;
    return FMOD_OK;
}

//
// Helper function that loads a bank in using the given method
//
FMOD_RESULT loadBank(FMOD::Studio::System* system, LoadBankMethod method, const char* filename, FMOD::Studio::Bank** bank)
{
    if (method == LoadBank_File)
    {
        return system->loadBankFile(filename, FMOD_STUDIO_LOAD_BANK_NONBLOCKING, bank);
    }
    else if (method == LoadBank_Memory || method == LoadBank_MemoryPoint)
    {
        char* memoryBase;
        char* memoryPtr;
        int memoryLength;
        FMOD_RESULT result = loadFile(filename, &memoryBase, &memoryPtr, &memoryLength);
        if (result != FMOD_OK)
        {
            return result;
        }

        FMOD_STUDIO_LOAD_MEMORY_MODE memoryMode = (method == LoadBank_MemoryPoint ? FMOD_STUDIO_LOAD_MEMORY_POINT : FMOD_STUDIO_LOAD_MEMORY);
        result = system->loadBankMemory(memoryPtr, memoryLength, memoryMode, FMOD_STUDIO_LOAD_BANK_NONBLOCKING, bank);
        if (result != FMOD_OK)
        {
            free(memoryBase);
            return result;
        }

        if (method == LoadBank_MemoryPoint)
        {
            // Keep memory around until bank unload completes
            result = (*bank)->setUserData(memoryBase);
        }
        else
        {
            // Don't need memory any more
            free(memoryBase);
        }
        return result;
    }
    else
    {
        // Set up custom callback
        FMOD_STUDIO_BANK_INFO info;
        memset(&info, 0, sizeof(info));
        info.size = sizeof(info);
        info.opencallback = customFileOpen;
        info.closecallback = customFileClose;
        info.readcallback = customFileRead;
        info.seekcallback = customFileSeek;
        info.userdata = (void*)filename;

        return system->loadBankCustom(&info, FMOD_STUDIO_LOAD_BANK_NONBLOCKING, bank);
    }
}

//
// Helper function to return state as a string
//
const char* getLoadingStateString(FMOD_STUDIO_LOADING_STATE state, FMOD_RESULT loadResult)
{
    switch (state)
    {
        case FMOD_STUDIO_LOADING_STATE_UNLOADING:
            return "unloading  ";
        case FMOD_STUDIO_LOADING_STATE_UNLOADED:
            return "unloaded   ";
        case FMOD_STUDIO_LOADING_STATE_LOADING:
            return "loading    ";
        case FMOD_STUDIO_LOADING_STATE_LOADED:
            return "loaded     ";
        case FMOD_STUDIO_LOADING_STATE_ERROR:
            // Show some common errors
            if (loadResult == FMOD_ERR_NOTREADY)
            {
                return "error (rdy)";
            }
            else if (loadResult == FMOD_ERR_FILE_BAD)
            {
                return "error (bad)";
            }
            else if (loadResult == FMOD_ERR_FILE_NOTFOUND)
            {
                return "error (mis)";
            }
            else
            {
                return "error      ";
            }
        default:
            return "???";
    };
}

//
// Helper function to return handle validity as a string.
// Just because the bank handle is valid doesn't mean the bank load
// has completed successfully!
//
const char* getHandleStateString(FMOD::Studio::Bank* bank)
{
    if (bank == NULL)
    {
        return "null   ";
    }
    else if (!bank->isValid())
    {
        return "invalid";
    }
    else
    {
        return "valid  ";
    }
}

//
// Callback to free memory-point allocation when it is safe to do so
//
FMOD_RESULT F_CALLBACK studioCallback(FMOD_STUDIO_SYSTEM *system, FMOD_STUDIO_SYSTEM_CALLBACK_TYPE type, void *commanddata, void *userdata)
{
    if (type == FMOD_STUDIO_SYSTEM_CALLBACK_BANK_UNLOAD)
    {
        // For memory-point, it is now safe to free our memory
        FMOD::Studio::Bank* bank = (FMOD::Studio::Bank*)commanddata;
        void* memory;
        ERRCHECK(bank->getUserData(&memory));
        if (memory)
        {
            free(memory);
        }
    }
    return FMOD_OK;
}

//
// Main example code
//
int FMOD_Main()
{
    void *extraDriverData = 0;
    Common_Init(&extraDriverData);

    FMOD::Studio::System* system;
    ERRCHECK( FMOD::Studio::System::create(&system) );
    ERRCHECK( system->initialize(1024, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, extraDriverData) );
    ERRCHECK( system->setCallback(studioCallback, FMOD_STUDIO_SYSTEM_CALLBACK_BANK_UNLOAD) );

    static const int BANK_COUNT = 4;
    static const char* BANK_NAMES[] =
    {
        "SFX.bank",
        "Music.bank",
        "Vehicles.bank",
        "VO.bank",
    };

    FMOD::Studio::Bank* banks[BANK_COUNT] = {0};
    bool wantBankLoaded[BANK_COUNT] = {0};
    bool wantSampleLoad = true;

    do
    {
        Common_Update();
        
        for (int i=0; i<BANK_COUNT; ++i)
        {
            if (Common_BtnPress((Common_Button)(BTN_ACTION1 + i)))
            {
                // Toggle bank load, or bank unload
                if (!wantBankLoaded[i])
                {
                    ERRCHECK(loadBank(system, (LoadBankMethod)i, Common_MediaPath(BANK_NAMES[i]), &banks[i]));
                    wantBankLoaded[i] = true;
                }
                else
                {
                    ERRCHECK(banks[i]->unload());
                    wantBankLoaded[i] = false;
                }
            }
        }
        if (Common_BtnPress(BTN_MORE))
        {
            wantSampleLoad = !wantSampleLoad;
        }

        // Load bank sample data automatically if that mode is enabled
        // Also query current status for text display
        FMOD_RESULT loadStateResult[BANK_COUNT] = { FMOD_OK, FMOD_OK, FMOD_OK, FMOD_OK,  };
        FMOD_RESULT sampleStateResult[BANK_COUNT] = { FMOD_OK, FMOD_OK, FMOD_OK, FMOD_OK, };
        FMOD_STUDIO_LOADING_STATE bankLoadState[BANK_COUNT] = { FMOD_STUDIO_LOADING_STATE_UNLOADED, FMOD_STUDIO_LOADING_STATE_UNLOADED, FMOD_STUDIO_LOADING_STATE_UNLOADED, FMOD_STUDIO_LOADING_STATE_UNLOADED };
        FMOD_STUDIO_LOADING_STATE sampleLoadState[BANK_COUNT] = { FMOD_STUDIO_LOADING_STATE_UNLOADED, FMOD_STUDIO_LOADING_STATE_UNLOADED, FMOD_STUDIO_LOADING_STATE_UNLOADED, FMOD_STUDIO_LOADING_STATE_UNLOADED };
        for (int i=0; i<BANK_COUNT; ++i)
        {
            if (banks[i] && banks[i]->isValid())
            {
                loadStateResult[i] = banks[i]->getLoadingState(&bankLoadState[i]);
            }
            if (bankLoadState[i] == FMOD_STUDIO_LOADING_STATE_LOADED)
            {
                sampleStateResult[i] = banks[i]->getSampleLoadingState(&sampleLoadState[i]);
                if (wantSampleLoad && sampleLoadState[i] == FMOD_STUDIO_LOADING_STATE_UNLOADED)
                {
                    ERRCHECK(banks[i]->loadSampleData());
                }
                else if (!wantSampleLoad && (sampleLoadState[i] == FMOD_STUDIO_LOADING_STATE_LOADING || sampleLoadState[i] == FMOD_STUDIO_LOADING_STATE_LOADED))
                {
                    ERRCHECK(banks[i]->unloadSampleData());
                }
            }
        }

        ERRCHECK( system->update() );

        Common_Draw("==================================================");
        Common_Draw("Bank Load Example.");
        Common_Draw("Copyright (c) Firelight Technologies 2012-2022.");
        Common_Draw("==================================================");
        Common_Draw("Name            Handle  Bank-State  Sample-State");

        for (int i=0; i<BANK_COUNT; ++i)
        {
            char namePad[64] = {0};
            int bankNameLen = strlen(BANK_NAMES[i]);
            memset(namePad, ' ', 15);
            strncpy(namePad, BANK_NAMES[i], bankNameLen);

            Common_Draw("%s %s %s %s",
                    namePad, 
                    getHandleStateString(banks[i]), 
                    getLoadingStateString(bankLoadState[i], loadStateResult[i]), 
                    getLoadingStateString(sampleLoadState[i], sampleStateResult[i]));
        }

        Common_Draw("");
        Common_Draw("Press %s to load bank 1 via %s",Common_BtnStr(BTN_ACTION1), BANK_LOAD_METHOD_NAMES[0]);
        Common_Draw("Press %s to load bank 2 via %s",Common_BtnStr(BTN_ACTION2), BANK_LOAD_METHOD_NAMES[1]);
        Common_Draw("Press %s to load bank 3 via %s",Common_BtnStr(BTN_ACTION3), BANK_LOAD_METHOD_NAMES[2]);
        Common_Draw("Press %s to load bank 4 via %s",Common_BtnStr(BTN_ACTION4), BANK_LOAD_METHOD_NAMES[3]);
        Common_Draw("Press %s to toggle sample data", Common_BtnStr(BTN_MORE));
        Common_Draw("Press %s to quit", Common_BtnStr(BTN_QUIT));

        Common_Sleep(50);
    } while (!Common_BtnPress(BTN_QUIT));
    
    ERRCHECK( system->unloadAll() );
    ERRCHECK( system->flushCommands() );
    ERRCHECK( system->release() );

    Common_Close();

    return 0;
}
