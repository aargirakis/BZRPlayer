#ifndef _sidid_h_
#define _sidid_h_

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __WIN32__
	#include <windows.h>
#endif
#define MAX_SIGSIZE 4096
#define MAX_PATHNAME 256
#define END -1
#define ANY -2
#define AND -3
#define NAME -4
typedef struct
{
	char *name;
	int count;
	void *firstsig;
	void *next;
} SIDID;

int readconfig(const char *name);
void identifyfile(char *name, char *fullname);
int identifybuffer(SIDID *id, unsigned char *buffer, int length);
char* identify(unsigned char *buffer, int length);
#ifdef __cplusplus
}
#endif
#endif // _sidid_h_
