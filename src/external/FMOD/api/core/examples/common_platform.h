/*==============================================================================
FMOD Example Framework
Copyright (c), Firelight Technologies Pty, Ltd 2012-2022.
==============================================================================*/
#include <windows.h>

int FMOD_Main();

#define COMMON_PLATFORM_SUPPORTS_FOPEN

#define Common_snprintf _snprintf
#define Common_vsnprintf _vsnprintf

void Common_TTY(const char *format, ...);

typedef CRITICAL_SECTION Common_Mutex;

inline void Common_Mutex_Create(Common_Mutex *mutex)
{
    InitializeCriticalSection(mutex);
}

inline void Common_Mutex_Destroy(Common_Mutex *mutex)
{
    DeleteCriticalSection(mutex);
}

inline void Common_Mutex_Enter(Common_Mutex *mutex)
{
    EnterCriticalSection(mutex);
}

inline void Common_Mutex_Leave(Common_Mutex *mutex)
{
    LeaveCriticalSection(mutex);
}

