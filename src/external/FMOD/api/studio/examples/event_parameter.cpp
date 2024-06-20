/*==============================================================================
Event Parameter Example
Copyright (c), Firelight Technologies Pty, Ltd 2012-2022.

This example demonstrates how to control event playback using game parameters.
==============================================================================*/
#include "fmod_studio.hpp"
#include "fmod.hpp"
#include "common.h"

int FMOD_Main()
{
    void *extraDriverData = NULL;
    Common_Init(&extraDriverData);

    FMOD::Studio::System* system = NULL;
    ERRCHECK( FMOD::Studio::System::create(&system) );
    ERRCHECK( system->initialize(1024, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, extraDriverData) );

    FMOD::Studio::Bank* masterBank = NULL;
    ERRCHECK( system->loadBankFile(Common_MediaPath("Master.bank"), FMOD_STUDIO_LOAD_BANK_NORMAL, &masterBank) );

    FMOD::Studio::Bank* stringsBank = NULL;
    ERRCHECK( system->loadBankFile(Common_MediaPath("Master.strings.bank"), FMOD_STUDIO_LOAD_BANK_NORMAL, &stringsBank) );

    FMOD::Studio::Bank* sfxBank = NULL;
    ERRCHECK( system->loadBankFile(Common_MediaPath("SFX.bank"), FMOD_STUDIO_LOAD_BANK_NORMAL, &sfxBank) );

    FMOD::Studio::EventDescription* eventDescription = NULL;
    ERRCHECK( system->getEvent("event:/Character/Player Footsteps", &eventDescription) );

    // Find the parameter once and then set by handle
    // Or we can just find by name every time but by handle is more efficient if we are setting lots of parameters
    FMOD_STUDIO_PARAMETER_DESCRIPTION paramDesc;
    ERRCHECK( eventDescription->getParameterDescriptionByName("Surface", &paramDesc) );
    FMOD_STUDIO_PARAMETER_ID surfaceID = paramDesc.id;

    FMOD::Studio::EventInstance* eventInstance = NULL;
    ERRCHECK( eventDescription->createInstance(&eventInstance) );

    // Make the event audible to start with
    float surfaceParameterValue = 1.0f;
    ERRCHECK( eventInstance->setParameterByID(surfaceID, surfaceParameterValue) );

    do
    {
        Common_Update();

        if (Common_BtnPress(BTN_MORE))
        {
            ERRCHECK( eventInstance->start() );
        }

        if (Common_BtnPress(BTN_ACTION1))
        {
            surfaceParameterValue = Common_Max(paramDesc.minimum, surfaceParameterValue - 1.0f);
            ERRCHECK( eventInstance->setParameterByID(surfaceID, surfaceParameterValue) );
        }

        if (Common_BtnPress(BTN_ACTION2))
        {
            surfaceParameterValue = Common_Min(surfaceParameterValue + 1.0f, paramDesc.maximum);
            ERRCHECK( eventInstance->setParameterByID(surfaceID, surfaceParameterValue) );
        }

        ERRCHECK( system->update() );

        float userValue = 0.0f;
        float finalValue = 0.0f;
        ERRCHECK( eventInstance->getParameterByID(surfaceID, &userValue, &finalValue) );

        Common_Draw("==================================================");
        Common_Draw("Event Parameter Example.");
        Common_Draw("Copyright (c) Firelight Technologies 2012-2022.");
        Common_Draw("==================================================");
        Common_Draw("");
        Common_Draw("Surface Parameter = (user: %1.1f, final: %1.1f)", userValue, finalValue);
        Common_Draw("");
        Common_Draw("Surface Parameter:");
        Common_Draw("Press %s to play event", Common_BtnStr(BTN_MORE));
        Common_Draw("Press %s to decrease value", Common_BtnStr(BTN_ACTION1));
        Common_Draw("Press %s to increase value", Common_BtnStr(BTN_ACTION2));
        Common_Draw("");
        Common_Draw("Press %s to quit", Common_BtnStr(BTN_QUIT));

        Common_Sleep(50);
    } while (!Common_BtnPress(BTN_QUIT));

    ERRCHECK( system->release() );

    Common_Close();

    return 0;
}
