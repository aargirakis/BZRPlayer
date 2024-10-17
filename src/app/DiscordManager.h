#ifndef DISCORDMANAGER_H
#define DISCORDMANAGER_H

#include "discord.h"

using namespace discord;

Core* init();
void updateActivity(Activity activity, Core* core);

#endif //DISCORDMANAGER_H
