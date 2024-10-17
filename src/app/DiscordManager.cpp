#include <iostream>

#include "discord.h"

using namespace std;
using namespace discord;

//TODO
void Log(LogLevel level, const char* message)
{
    cout << "Log(" << (int)level << "): " << message << "\n";
}

//TODO DiscordCreateFlags_NoRequireDiscord

//TODO destroy function
// discord.Dispose();

Core* init()
{
    Core* core{};
    auto response = Core::Create(1297172338877403136, DiscordCreateFlags_Default, &core);

    if (core)
    {
        cout << "Discord successfully initalizated" << endl;
    }
    else
    {
        cout << "Failed to instantiate Discord " << (int)response << endl;
    }
    return core;
}

void updateActivity(Activity activity, Core* core)
{
    core->ActivityManager().UpdateActivity(activity, [](Result result)
    {
        if (result != Result::Ok)
        {
            cerr << "Activity updating error: " << static_cast<int>(result) << endl;
        }
        else
        {
            cout << "DISCORD ACTIVITY UPDATED" << endl;
        }
    });

    core->RunCallbacks();
    //this_thread::sleep_for(chrono::seconds(15));
}
