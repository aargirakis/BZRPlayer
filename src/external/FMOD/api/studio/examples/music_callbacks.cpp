/*==============================================================================
Music Callback Example
Copyright (c), Firelight Technologies Pty, Ltd 2012-2022.

This example demonstrates beat and named marker callbacks when playing music.

### See Also ###
* Studio::EventInstance::setCallback
* FMOD_STUDIO_EVENT_CALLBACK_TIMELINE_MARKER
* FMOD_STUDIO_EVENT_CALLBACK_TIMELINE_BEAT
==============================================================================*/
#include "fmod_studio.hpp"
#include "fmod.hpp"
#include "common.h"
#include <vector>
#include <string>

static const int MAX_ENTRIES = 6;

struct CallbackInfo
{
    Common_Mutex mMutex;
    std::vector<std::string> mEntries;
};

FMOD_RESULT F_CALLBACK markerCallback(FMOD_STUDIO_EVENT_CALLBACK_TYPE type, FMOD_STUDIO_EVENTINSTANCE* event, void *parameters);

int FMOD_Main()
{
    void *extraDriverData = NULL;
    Common_Init(&extraDriverData);

    FMOD::Studio::System* system = NULL;
    ERRCHECK( FMOD::Studio::System::create(&system) );

    // The example Studio project is authored for 5.1 sound, so set up the system output mode to match
    FMOD::System* coreSystem = NULL;
    ERRCHECK( system->getCoreSystem(&coreSystem) );
    ERRCHECK( coreSystem->setSoftwareFormat(0, FMOD_SPEAKERMODE_5POINT1, 0) );

    ERRCHECK( system->initialize(1024, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, extraDriverData) );

    FMOD::Studio::Bank* masterBank = NULL;
    ERRCHECK( system->loadBankFile(Common_MediaPath("Master.bank"), FMOD_STUDIO_LOAD_BANK_NORMAL, &masterBank) );

    FMOD::Studio::Bank* stringsBank = NULL;
    ERRCHECK( system->loadBankFile(Common_MediaPath("Master.strings.bank"), FMOD_STUDIO_LOAD_BANK_NORMAL, &stringsBank) );

    FMOD::Studio::Bank* musicBank = NULL;
    FMOD_RESULT result = system->loadBankFile(Common_MediaPath("Music.bank"), FMOD_STUDIO_LOAD_BANK_NORMAL, &musicBank);
    if (result != FMOD_OK)
    {
        // Music bank is not exported by default, you will have to export from the tool first
        Common_Fatal("Please export music.bank from the Studio tool to run this example");
    }

    FMOD::Studio::EventDescription* eventDescription = NULL;
    ERRCHECK( system->getEvent("event:/Music/Level 01", &eventDescription) );

    FMOD::Studio::EventInstance* eventInstance = NULL;
    ERRCHECK( eventDescription->createInstance(&eventInstance) );
    
    CallbackInfo info;
    Common_Mutex_Create(&info.mMutex);

    ERRCHECK( eventInstance->setUserData(&info) );
    ERRCHECK( eventInstance->setCallback(markerCallback,
            FMOD_STUDIO_EVENT_CALLBACK_TIMELINE_MARKER | FMOD_STUDIO_EVENT_CALLBACK_TIMELINE_BEAT |
            FMOD_STUDIO_EVENT_CALLBACK_SOUND_PLAYED | FMOD_STUDIO_EVENT_CALLBACK_SOUND_STOPPED) );
    ERRCHECK( eventInstance->start() );

    FMOD_STUDIO_PARAMETER_DESCRIPTION parameterDescription;
    ERRCHECK( eventDescription->getParameterDescriptionByName("Progression", &parameterDescription) );

    FMOD_STUDIO_PARAMETER_ID progressionID = parameterDescription.id;

    float progression = 0.0f;
    ERRCHECK(eventInstance->setParameterByID(progressionID, progression));

    do
    {
        Common_Update();

        if (Common_BtnPress(BTN_MORE))
        {
            progression = (progression == 0.0f ? 1.0f : 0.0f);
            ERRCHECK(eventInstance->setParameterByID(progressionID, progression));
        }

        ERRCHECK( system->update() );

        int position;
        ERRCHECK( eventInstance->getTimelinePosition(&position) );

        Common_Draw("==================================================");
        Common_Draw("Music Callback Example.");
        Common_Draw("Copyright (c) Firelight Technologies 2012-2022.");
        Common_Draw("==================================================");
        Common_Draw("");
        Common_Draw("Timeline = %d", position);
        Common_Draw("");
        // Obtain lock and look at our strings
        Common_Mutex_Enter(&info.mMutex);
        for (size_t i=0; i<info.mEntries.size(); ++i)
        {
            Common_Draw("  %s\n", info.mEntries[i].c_str());
        }
        Common_Mutex_Leave(&info.mMutex);
        Common_Draw("");
        Common_Draw("Press %s to toggle progression (currently %g)", Common_BtnStr(BTN_MORE), progression);
        Common_Draw("Press %s to quit", Common_BtnStr(BTN_QUIT));

        Common_Sleep(50);
    } while (!Common_BtnPress(BTN_QUIT));

    ERRCHECK( system->release() );

    Common_Mutex_Destroy(&info.mMutex);
    Common_Close();

    return 0;
}

// Obtain a lock and add a string entry to our list
void markerAddString(CallbackInfo* info, const char* format, ...)
{
    char buf[256];
    va_list args;
    va_start(args, format);
    Common_vsnprintf(buf, 256, format, args);
    va_end(args);
    buf[255] = '\0';
    Common_Mutex_Enter(&info->mMutex);
    if (info->mEntries.size() >= MAX_ENTRIES)
    {
        info->mEntries.erase(info->mEntries.begin());
    }
    info->mEntries.push_back(std::string(buf));
    Common_Mutex_Leave(&info->mMutex);
}

// Callback from Studio - Remember these callbacks will occur in the Studio update thread, NOT the game thread.
FMOD_RESULT F_CALLBACK markerCallback(FMOD_STUDIO_EVENT_CALLBACK_TYPE type, FMOD_STUDIO_EVENTINSTANCE* event, void *parameters)
{
    CallbackInfo* callbackInfo;
    ERRCHECK(((FMOD::Studio::EventInstance*)event)->getUserData((void**)&callbackInfo));

    if (type == FMOD_STUDIO_EVENT_CALLBACK_TIMELINE_MARKER)
    {
        FMOD_STUDIO_TIMELINE_MARKER_PROPERTIES* props = (FMOD_STUDIO_TIMELINE_MARKER_PROPERTIES*)parameters;
        markerAddString(callbackInfo, "Named marker '%s'", props->name);
    }
    else if (type == FMOD_STUDIO_EVENT_CALLBACK_TIMELINE_BEAT)
    {
        FMOD_STUDIO_TIMELINE_BEAT_PROPERTIES* props = (FMOD_STUDIO_TIMELINE_BEAT_PROPERTIES*)parameters;
        markerAddString(callbackInfo, "beat %d, bar %d (tempo %.1f %d:%d)", props->beat, props->bar, props->tempo, props->timesignatureupper, props->timesignaturelower);
    }
    if (type == FMOD_STUDIO_EVENT_CALLBACK_SOUND_PLAYED || type == FMOD_STUDIO_EVENT_CALLBACK_SOUND_STOPPED)
    {
        FMOD::Sound* sound = (FMOD::Sound*)parameters;
        char name[256];
        ERRCHECK(sound->getName(name, 256));
        unsigned int len;
        ERRCHECK(sound->getLength(&len, FMOD_TIMEUNIT_MS));

        markerAddString(callbackInfo, "Sound '%s' (length %.3f) %s", 
                name, (float)len/1000.0f, 
                type == FMOD_STUDIO_EVENT_CALLBACK_SOUND_PLAYED ? "Started" : "Stopped");
    }

    return FMOD_OK;
}
