#if !defined(win32_types_h)
#define win32_types_h

#ifdef __amigaos4__
#define OLD_TYPEDEFS
#include <exec/types.h>
#endif

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

//#ifndef __amigaos4__
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int  uint32;
typedef unsigned long long int uint64;
//#endif

typedef signed char sint8;
typedef signed short sint16;
typedef signed int sint32;

//#ifndef __amigaos4__
typedef signed char int8;
typedef signed short int16;
typedef signed int int32;
typedef signed long long int int64;
//#endif

#ifdef __amigaos4__
typedef float float32;
typedef double float64;
#endif

//added by blazer
typedef unsigned int guint32;
typedef unsigned short guint16;
typedef unsigned char guint8;
typedef int gint;
typedef unsigned int guint;

typedef char   gchar;


#endif // win32_types_h
