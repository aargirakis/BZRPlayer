/*==============================================================================
Event 3D Multi-Listener Example
Copyright (c), Firelight Technologies Pty, Ltd 2012-2022.

This example demonstrates how use listener weighting to crossfade listeners
in and out.
==============================================================================*/
#include "fmod_studio.hpp"
#include "fmod.hpp"
#include "common.h"

const int SCREEN_WIDTH = NUM_COLUMNS;
const int SCREEN_HEIGHT = 10;

char backBuffer[(SCREEN_WIDTH + 1) * SCREEN_HEIGHT + 1] = {0};
char screenBuffer[(SCREEN_WIDTH + 1) * SCREEN_HEIGHT + 1] = {0};

void initializeScreenBuffer();
void updateScreenPosition(const FMOD_VECTOR& worldPosition, float listenerDist, float weight1, float weight2);

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

    FMOD::Studio::Bank* vehiclesBank = NULL;
    ERRCHECK( system->loadBankFile(Common_MediaPath("Vehicles.bank"), FMOD_STUDIO_LOAD_BANK_NORMAL, &vehiclesBank) );
    
    FMOD::Studio::EventDescription* eventDescription = NULL;
    ERRCHECK( system->getEvent("event:/Vehicles/Ride-on Mower", &eventDescription) );

    FMOD::Studio::EventInstance* eventInstance = NULL;
    ERRCHECK( eventDescription->createInstance(&eventInstance) );

    ERRCHECK( eventInstance->setParameterByName("RPM", 650.0f) );
    ERRCHECK( eventInstance->start() );

    // Position two listeners
    ERRCHECK( system->setNumListeners(2) );
    int activeListener = 0;
    float listenerDist = 8.0f;
    float listenerWeight[2] = { 1.0f, 0.0f };
    FMOD_3D_ATTRIBUTES listenerAttributes[2] = {};
    listenerAttributes[0].forward.z = 1.0f;
    listenerAttributes[0].up.y = 1.0f;
    listenerAttributes[0].position.x = -listenerDist;
    listenerAttributes[1].forward.z = 1.0f;
    listenerAttributes[1].up.y = 1.0f;
    listenerAttributes[1].position.x = listenerDist;

    ERRCHECK( system->setListenerAttributes(0, &listenerAttributes[0]) );
    ERRCHECK( system->setListenerWeight(0, listenerWeight[0]) );
    ERRCHECK( system->setListenerAttributes(1, &listenerAttributes[1]) );
    ERRCHECK( system->setListenerWeight(1, listenerWeight[1]) );

    // Position the event 2 units in front of the listener
    FMOD_3D_ATTRIBUTES carAttributes = {};
    carAttributes.forward.z = 1.0f;
    carAttributes.up.y = 1.0f;
    carAttributes.position.x = 0.0f;
    carAttributes.position.z = 2.0f;
    ERRCHECK( eventInstance->set3DAttributes(&carAttributes) );

    initializeScreenBuffer();

    do
    {
        Common_Update();
        
        if (Common_BtnPress(BTN_LEFT))
        {
            carAttributes.position.x -= 1.0f;
            ERRCHECK( eventInstance->set3DAttributes(&carAttributes) );
        }
        
        if (Common_BtnPress(BTN_RIGHT))
        {
            carAttributes.position.x += 1.0f;
            ERRCHECK( eventInstance->set3DAttributes(&carAttributes) );
        }
        
        if (Common_BtnPress(BTN_UP))
        {
            carAttributes.position.z += 1.0f;
            ERRCHECK( eventInstance->set3DAttributes(&carAttributes) );
        }
        
        if (Common_BtnPress(BTN_DOWN))
        {
            carAttributes.position.z -= 1.0f;
            ERRCHECK( eventInstance->set3DAttributes(&carAttributes) );
        }

        if (Common_BtnPress(BTN_ACTION1))
        {
            activeListener++;
            if (activeListener > 2)
                activeListener = 0;
        }

        if (Common_BtnPress(BTN_ACTION2))
        {
            activeListener--;
            if (activeListener < 0)
                activeListener = 2;
        }

        if (Common_BtnPress(BTN_ACTION3))
        {
            listenerDist -= 1.0f;
            if (listenerDist < 0.0f)
                listenerDist = 0.0f;
        }

        if (Common_BtnPress(BTN_ACTION4))
        {
            listenerDist += 1.0f;
            if (listenerDist < 0.0f)
                listenerDist = 0.0f;
        }

        for (int i=0; i<2; ++i)
        {
            float target = (activeListener == i || activeListener == 2); // 0 = left, 1 = right, 2 = both

            float dist = (target - listenerWeight[i]);
            float step = 50.0f / 1000.0f; // very rough estimate of 50ms per update, not properly timed

            if (dist >= -step && dist <= step)
                listenerWeight[i] = target;
            else if (dist > 0.0f)
                listenerWeight[i] += step;
            else
                listenerWeight[i] += -step;
        }

        listenerAttributes[0].position.x = -listenerDist;
        listenerAttributes[1].position.x = listenerDist;
        ERRCHECK( system->setListenerAttributes(0, &listenerAttributes[0]) );
        ERRCHECK( system->setListenerAttributes(1, &listenerAttributes[1]) );
        ERRCHECK( system->setListenerWeight(0, listenerWeight[0]) );
        ERRCHECK( system->setListenerWeight(1, listenerWeight[1]) );

        ERRCHECK( system->update() );

        updateScreenPosition(carAttributes.position, listenerDist, listenerWeight[0], listenerWeight[1]);

        Common_Draw("==================================================");
        Common_Draw("Event 3D Multi-Listener Example.");
        Common_Draw("Copyright (c) Firelight Technologies 2012-2022.");
        Common_Draw("==================================================");
        Common_Draw(screenBuffer);

        Common_Draw("Left listener: %d%%", (int)(listenerWeight[0] * 100));
        Common_Draw("Right listener: %d%%", (int)(listenerWeight[1] * 100));
        Common_Draw("Use the arrow keys (%s, %s, %s, %s) to control the event position", 
            Common_BtnStr(BTN_LEFT), Common_BtnStr(BTN_RIGHT), Common_BtnStr(BTN_UP), Common_BtnStr(BTN_DOWN));
        Common_Draw("Use %s and %s to toggle left/right/both listeners", Common_BtnStr(BTN_ACTION1), Common_BtnStr(BTN_ACTION2));
        Common_Draw("Use %s and %s to move listeners closer or further apart", Common_BtnStr(BTN_ACTION3), Common_BtnStr(BTN_ACTION4));
        Common_Draw("Press %s to quit", Common_BtnStr(BTN_QUIT));

        Common_Sleep(50);
    } while (!Common_BtnPress(BTN_QUIT));

    ERRCHECK( system->release() );

    Common_Close();

    return 0;
}

void initializeScreenBuffer()
{
    memset(backBuffer, ' ', sizeof(backBuffer));

    int idx = SCREEN_WIDTH;
    for (int i = 0; i < SCREEN_HEIGHT; ++i)
    {
        backBuffer[idx] = '\n';
        idx += SCREEN_WIDTH + 1;
    }

    backBuffer[(SCREEN_WIDTH + 1) * SCREEN_HEIGHT] = '\0';
    memcpy(screenBuffer, backBuffer, sizeof(screenBuffer));
}

void setCharacterIndex(const FMOD_VECTOR& position, char ch)
{
    int row = static_cast<int>(-position.z + (SCREEN_HEIGHT / 2));
    int col = static_cast<int>(position.x + (SCREEN_WIDTH / 2));
    
    if (0 < row && row < SCREEN_HEIGHT && 0 < col && col < SCREEN_WIDTH)
    {
        screenBuffer[row * (SCREEN_WIDTH + 1) + col] = ch;
    }
}

char symbolForWeight(float weight)
{
    if (weight >= 0.95f)
        return 'X';
    else if (weight >= 0.05f)
        return 'x';
    else
        return '.';
}

void updateScreenPosition(const FMOD_VECTOR& worldPosition, float listenerDist, float weight1, float weight2)
{
    memcpy(screenBuffer, backBuffer, sizeof(screenBuffer));

    FMOD_VECTOR pos = {0};
    setCharacterIndex(pos, '^');

    pos.x = -listenerDist;
    setCharacterIndex(pos, symbolForWeight(weight1));

    pos.x = listenerDist;
    setCharacterIndex(pos, symbolForWeight(weight2));

    setCharacterIndex(worldPosition, 'o');
}
