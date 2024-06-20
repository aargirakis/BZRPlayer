/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2013 Leandro Nini <drfiemost@users.sourceforge.net>
 * Copyright 2007-2010 Antti Lankila
 * Copyright 2000 Simon White
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef SIDPLAYFP_H
#define SIDPLAYFP_H

#include <stdint.h>
#include <stdio.h>

#include "sidplayfp/siddefs.h"
#include "sidplayfp/sidversion.h"

class  SidConfig;
class  SidTune;
class  SidInfo;
class  EventContext;

// Private Sidplayer
namespace SIDPLAYFP_NAMESPACE
{
    class Player;
}

/**
 * sidplayfp
 */
class SID_EXTERN sidplayfp
{
private:
    SIDPLAYFP_NAMESPACE::Player &sidplayer;

public:
    sidplayfp();
    ~sidplayfp();

    /**
     * Get the current engine configuration.
     *
     * @return a const reference to the current configuration.
     */
    const SidConfig &config() const;

    /**
     * Get the current player informations.
     *
     * @return a const reference to the current info.
     */
    const SidInfo &info() const;

    /**
     * Configure the engine.
     * Check #error for detailed message if something goes wrong.
     *
     * @param cfg the new configuration
     * @return true on success, false otherwise.
     */
    bool config(const SidConfig &cfg);

    /**
     * Error message.
     *
     * @return string error message.
     */
    const char *error() const;

    /**
     * Load a tune.
     * Check #error for detailed message if something goes wrong.
     *
     * @param tune the SidTune to load, 0 unloads current tune.
     * @return true on sucess, false otherwise.
     */
    bool load(SidTune *tune);

    /**
     * Produce samples to play.
     *
     * @param buffer pointer to the buffer to fill with samples.
     * @param count the size of the buffer measured in 16 bit samples.
     * @return the number of produced samples.
     */
    uint_least32_t play(short *buffer, uint_least32_t count);

    /**
     * Check if the engine is playing or stopped.
     *
     * @return true if playing, false otherwise.
     */
    bool isPlaying() const;

    /** Stop the engine. */
    void stop();

    /**
     * Control debugging.
     * Only has effect if library have been compiled
     * with the --enable-debug option.
     *
     * @param enable enable/disable debugging.
     * @param out the file where to redirect the debug info.
     */
    void debug(bool enable, FILE *out);

    /**
     * Mute/unmute a SID channel.
     *
     * @param sidNum the SID chip, 0 for the first one, 1 for the second.
     * @param voice the channel to mute/unmute.
     * @param enable true unmutes the channel, false mutes it.
     */
    void mute(unsigned int sidNum, unsigned int voice, bool enable);

    /**
     *  Get state of SID
     *  @param freqs pointer to frequencies array
     *  @param levels pointer to levels array
     *  @return used size of freqs/levels (3 or 6 usually)
     */
    unsigned int getState(unsigned int* freqs, unsigned int* levels) const;

    /**
     * Get the current playing time with respect to resolution returned by timebase.
     *
     * @return the current playing time.
     */
    uint_least32_t time() const;

    /**
     * Set ROMs.
     * The ROMs are validate against known ones.
     *
     * @param kernal pointer to Kernal ROM.
     * @param basic pointer to Basic ROM, generally needed only for BASIC tunes.
     * @param character pointer to character generator ROM.
     */
    void setRoms(const uint8_t* kernal, const uint8_t* basic=0, const uint8_t* character=0);

    /**
     * Get the event scheduler.
     *
     * @return pointer to the scheduler.
     */
    EventContext *getEventContext();

    /**
     * Get the CIA 1 Timer A programmed value.
     */
    uint_least16_t getCia1TimerA() const;

    /**
    * Get CPU clock rate
    */
    uint_least32_t getCPUFreq() const;
};

#endif // SIDPLAYFP_H
