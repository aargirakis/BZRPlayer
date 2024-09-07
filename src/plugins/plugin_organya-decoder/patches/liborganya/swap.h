//
//  swap.h
//  org2dat
//
//  Created by Vincent Spader on 6/21/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#include <inttypes.h>

static uint16_t org_btoh_16(uint16_t val) {
    return (((uint16_t) (val) & (uint16_t) 0x00ffU) << 8) | (((uint16_t) (val) & (uint16_t) 0xff00U) >> 8);
}
//#define org_btoh_32(x) pfc::byteswap_if_le_t(x)

#define org_ltoh_16(x) x
#define org_ltoh_32(x) x
