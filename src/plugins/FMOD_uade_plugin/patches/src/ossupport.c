#include "ossupport.h"

#define EMSCRIPTEN 1
#ifdef EMSCRIPTEN
#include "standalonesupport.c"
#else
#include "unixsupport.c"
#endif