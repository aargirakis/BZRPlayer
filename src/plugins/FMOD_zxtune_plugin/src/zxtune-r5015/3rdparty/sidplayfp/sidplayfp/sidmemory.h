/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2012-2013 Leandro Nini <drfiemost@users.sourceforge.net>
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

#ifndef SIDMEMORY_H
#define SIDMEMORY_H

#include <stdint.h>

/**
 * An interface that allows access to c64 memory
 * for loading tunes and apply sid specific hacks.
 */
class sidmemory
{
public:
    /**
     * Read one byte from memory.
     *
     * @param addr the memory location from which to read from
     */
    virtual uint8_t readMemByte(uint_least16_t addr) =0;

    /**
     * Read two contiguous bytes from memory.
     *
     * @param addr the memory location from which to read from
     */
    virtual uint_least16_t readMemWord(uint_least16_t addr) =0;

    /**
     * Write one byte to memory.
     *
     * @param addr the memory location where to write
     * @param value the value to write
     */
    virtual void writeMemByte(uint_least16_t addr, uint8_t value) =0;

    /**
     * Write two contiguous bytes to memory.
     *
     * @param addr the memory location where to write
     * @param value the value to write
     */
    virtual void writeMemWord(uint_least16_t addr, uint_least16_t value) =0;

    /**
     * Fill ram area with a constant value.
     *
     * @param start the start of memory location where to write
     * @param value the value to write
     * @param size the number of bytes to fill
     */
    virtual void fillRam(uint_least16_t start, uint8_t value, unsigned int size) =0;

    /**
     * Copy a buffer into a ram area.
     *
     * @param start the start of memory location where to write
     * @param source the source buffer
     * @param size the number of bytes to copy
     */
    virtual void fillRam(uint_least16_t start, const uint8_t* source, unsigned int size) =0;

    /**
     * Change the RESET vector.
     *
     * @param addr the new addres to point to
     */
    virtual void installResetHook(uint_least16_t addr) =0;

    /**
     * Set BASIC Warm Start address.
     *
     * @param addr the new addres to point to
     */
    virtual void installBasicTrap(uint_least16_t addr) =0;

    /**
     * Set the start tune.
     *
     * @param tune the tune number
     */
    virtual void setBasicSubtune(uint8_t tune) =0;

    /** Use small granularity to
     * 1) avoid redundand ZeroRam checks for first bank
     * 2) avoid bank mapping for every operation
     **/
    static const unsigned READ_BANK_GRANULARITY = 2;
    static const unsigned WRITE_BANK_GRANULARITY = 4096;

    /**
     * Access memory as seen by CPU.
     *
     * Make it non-virtual to speedup due to polymorphism disabling and possible inlining.
     *
     * @param addr the address where to read from
     * @return value at address
     */
    uint8_t cpuRead(uint_least16_t addr);

    /**
     * Access memory as seen by CPU.
     *
     * @param addr the address where to write
     * @param data the value to write
     */
    virtual void cpuWrite(uint_least16_t addr, uint8_t data) = 0;

protected:
    virtual ~sidmemory() {}
};

#endif
