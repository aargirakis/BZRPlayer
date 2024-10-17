#include "discord.h"
#include "various.h"

using namespace std;
using namespace discord;

// TODO DiscordCreateFlags_NoRequireDiscord

// TODO destroy function
// discord.Dispose();

Core *init() {
    Core *core{};
    auto response = Core::Create(1297172338877403136, DiscordCreateFlags_Default, &core);

    if (core) {
        logDebug("Discord successfully initialized", "DiscordManager"); //TODO use getClassName() instead for all logs
    } else {
        logError("Failed to instantiate Discord with error " + static_cast<int>(response), "DiscordManager");
    }

    return core;
}

void updateActivity(const Activity &activity, Core *core) {
    core->ActivityManager().UpdateActivity(activity, [](Result result) {
        if (result == Result::Ok) {
            logDebug("DISCORD ACTIVITY UPDATED", "DiscordManager");
        } else {
            logError("Activity updating error " + static_cast<int>(result), "DiscordManager");
        }
    });

    core->RunCallbacks();
    //this_thread::sleep_for(chrono::seconds(15));
}
