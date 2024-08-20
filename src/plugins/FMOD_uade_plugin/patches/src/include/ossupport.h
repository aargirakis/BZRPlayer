#ifndef _UADE_OSSUPPORT_H_
#define _UADE_OSSUPPORT_H_

#define EMSCRIPTEN 1 //added by blazer
#ifdef EMSCRIPTEN
#include "standalonesupport.h"
#else
#include "unixsupport.h"
#endif

#include <string.h>
#endif


