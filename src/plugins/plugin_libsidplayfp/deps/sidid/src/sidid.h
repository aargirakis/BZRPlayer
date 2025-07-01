#ifndef SIDID_H
#define SIDID_H

#ifdef __cplusplus
extern "C" {
#endif

char *identifyBufferFromConfig(const char *sididCfgPath, unsigned char *buffer, int length);

#ifdef __cplusplus
}
#endif

#endif // SIDID_H
