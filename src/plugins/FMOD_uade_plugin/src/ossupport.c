#include "ossupport.h"

#define EMSCRIPTEN 1 //added by blazer
#ifdef EMSCRIPTEN
#include "standalonesupport.c"
#else
#include "unixsupport.c"
#endif