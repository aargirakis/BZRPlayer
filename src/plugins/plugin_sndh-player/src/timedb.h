#ifndef TIMEDB_H
#define TIMEDB_H

#include <vector>

using namespace std;

constexpr unsigned int HBIT = 32; /* # of bit for hash     */
constexpr unsigned int TBIT = 6; /* # of bit for track    */
constexpr unsigned int WBIT = 6; /* # of bit for hardware */

#define FBIT (64-HBIT-TBIT-WBIT) /* # of bit for frames   */
#define HFIX (32-HBIT)

#define TIMEDB_ENTRY(HASH,TRACK,FRAMES,FLAGS) \
{ 0x##HASH>>HFIX, TRACK-1, FLAGS, FRAMES }

#define E_EMPTY { 0,0,0,0 }

constexpr unsigned int STE = 0;
constexpr unsigned int YM = 0;
constexpr unsigned int TA = 0;
constexpr unsigned int TB = 0;
constexpr unsigned int TC = 0;
constexpr unsigned int TD = 0;

using dbentry_t = struct {
    unsigned int hash: HBIT; /* hash code              */
    unsigned int track: TBIT; /* track number (0-based) */
    unsigned int flags: WBIT; /* see enum               */
    unsigned int frames: FBIT; /* length in frames       */
};

static vector<dbentry_t> s_db = {

#include "timedb.inc.h"
};

#endif // TIMEDB_H
