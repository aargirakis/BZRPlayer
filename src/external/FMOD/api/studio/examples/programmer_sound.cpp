/*==============================================================================
Programmer Sound Example
Copyright (c), Firelight Technologies Pty, Ltd 2012-2022.

This example demonstrates how to implement the programmer sound callback to
play an event that has a programmer specified sound.

### See Also ###
Studio::EventInstance::setCallback
==============================================================================*/
#include "fmod_studio.hpp"
#include "fmod.hpp"
#include "common.h"

struct ProgrammerSoundContext
{
    FMOD::System* coreSystem;
    FMOD::Studio::System* system;
    const char* dialogueString;
};

FMOD_RESULT F_CALLBACK programmerSoundCallback(FMOD_STUDIO_EVENT_CALLBACK_TYPE type, FMOD_STUDIO_EVENTINSTANCE* event, void *parameters);

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

    FMOD::Studio::Bank* sfxBank = NULL;
    ERRCHECK( system->loadBankFile(Common_MediaPath("SFX.bank"), FMOD_STUDIO_LOAD_BANK_NORMAL, &sfxBank) );

    // Available banks
    unsigned int bankIndex = 0;
    static const char* const banks[] = { "Dialogue_EN.bank", "Dialogue_JP.bank", "Dialogue_CN.bank" };

    FMOD::Studio::Bank* localizedBank = NULL;
    ERRCHECK( system->loadBankFile(Common_MediaPath(banks[bankIndex]), FMOD_STUDIO_LOAD_BANK_NORMAL, &localizedBank) );

    FMOD::Studio::EventDescription* eventDescription = NULL;
    ERRCHECK( system->getEvent("event:/Character/Dialogue", &eventDescription) );

    FMOD::Studio::EventInstance* eventInstance = NULL;
    ERRCHECK( eventDescription->createInstance(&eventInstance) );

    // Dialogue keys available
    // These keys are shared amongst all audio tables
    unsigned int dialogueIndex = 0;
    static const char* const dialogue[] = {"welcome", "main menu", "goodbye"};

    ProgrammerSoundContext programmerSoundContext;
    programmerSoundContext.system = system;
    programmerSoundContext.coreSystem = coreSystem;
    programmerSoundContext.dialogueString = dialogue[dialogueIndex];

    ERRCHECK( eventInstance->setUserData(&programmerSoundContext) );
    ERRCHECK( eventInstance->setCallback(programmerSoundCallback, FMOD_STUDIO_EVENT_CALLBACK_CREATE_PROGRAMMER_SOUND | FMOD_STUDIO_EVENT_CALLBACK_DESTROY_PROGRAMMER_SOUND) );

    do
    {
        Common_Update();

        if (Common_BtnPress(BTN_ACTION1))
        {
            ERRCHECK( localizedBank->unload() );

            bankIndex = (bankIndex < 2) ? bankIndex + 1 : 0;
            ERRCHECK( system->loadBankFile(Common_MediaPath(banks[bankIndex]), FMOD_STUDIO_LOAD_BANK_NORMAL, &localizedBank) );
        }

        if (Common_BtnPress(BTN_ACTION2))
        {
            dialogueIndex = (dialogueIndex < 2) ? dialogueIndex + 1 : 0;
            programmerSoundContext.dialogueString = dialogue[dialogueIndex];
        }

        if (Common_BtnPress(BTN_MORE))
        {
            ERRCHECK( eventInstance->start() );
        }

        ERRCHECK( system->update() );

        Common_Draw("==================================================");
        Common_Draw("Programmer Sound Example.");
        Common_Draw("Copyright (c) Firelight Technologies 2012-2022.");
        Common_Draw("==================================================");
        Common_Draw("");
        Common_Draw("Press %s to change language", Common_BtnStr(BTN_ACTION1));
        Common_Draw("Press %s to change dialogue", Common_BtnStr(BTN_ACTION2));
        Common_Draw("Press %s to play the event",  Common_BtnStr(BTN_MORE));
        Common_Draw("");
        Common_Draw("Language:");
        Common_Draw("  %s English",  bankIndex == 0 ? ">" : " ");
        Common_Draw("  %s Japanese", bankIndex == 1 ? ">" : " ");
        Common_Draw("  %s Chinese",  bankIndex == 2 ? ">" : " ");
        Common_Draw("");
        Common_Draw("Dialogue:");
        Common_Draw("  %s Welcome to the FMOD Studio tutorial", dialogueIndex == 0 ? ">" : " ");
        Common_Draw("  %s This is the main menu",               dialogueIndex == 1 ? ">" : " ");
        Common_Draw("  %s Goodbye",                             dialogueIndex == 2 ? ">" : " ");
        Common_Draw("");
        Common_Draw("Press %s to quit", Common_BtnStr(BTN_QUIT));

        Common_Sleep(50);
    } while (!Common_BtnPress(BTN_QUIT));

    ERRCHECK( system->release() );

    Common_Close();

    return 0;
}

#define CHECK_RESULT(op) \
    { \
        FMOD_RESULT res = (op); \
        if (res != FMOD_OK) \
        { \
            return res; \
        } \
    }

FMOD_RESULT F_CALLBACK programmerSoundCallback(FMOD_STUDIO_EVENT_CALLBACK_TYPE type, FMOD_STUDIO_EVENTINSTANCE* event, void *parameters)
{
    FMOD::Studio::EventInstance* eventInstance = (FMOD::Studio::EventInstance*)event;

    if (type == FMOD_STUDIO_EVENT_CALLBACK_CREATE_PROGRAMMER_SOUND)
    {
        FMOD_STUDIO_PROGRAMMER_SOUND_PROPERTIES* props = (FMOD_STUDIO_PROGRAMMER_SOUND_PROPERTIES*)parameters;

        // Get our context from the event instance user data
        ProgrammerSoundContext* context = NULL;
        CHECK_RESULT( eventInstance->getUserData((void**)&context) );

        // Find the audio file in the audio table with the key
        FMOD_STUDIO_SOUND_INFO info;
        CHECK_RESULT( context->system->getSoundInfo(context->dialogueString, &info) );

        FMOD::Sound* sound = NULL;
        CHECK_RESULT( context->coreSystem->createSound(info.name_or_data, FMOD_LOOP_NORMAL | FMOD_CREATECOMPRESSEDSAMPLE | FMOD_NONBLOCKING | info.mode, &info.exinfo, &sound) );

        // Pass the sound to FMOD
        props->sound = (FMOD_SOUND*)sound;
        props->subsoundIndex = info.subsoundindex;
    }
    else if (type == FMOD_STUDIO_EVENT_CALLBACK_DESTROY_PROGRAMMER_SOUND)
    {
        FMOD_STUDIO_PROGRAMMER_SOUND_PROPERTIES* props = (FMOD_STUDIO_PROGRAMMER_SOUND_PROPERTIES*)parameters;

        // Obtain the sound
        FMOD::Sound* sound = (FMOD::Sound*)props->sound;

        // Release the sound
        CHECK_RESULT( sound->release() );
    }

    return FMOD_OK;
}