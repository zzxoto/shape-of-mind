#include "common.h"

typedef void (* fn_setTimeoutCallback)(void *);
void ut_setTimeout(fn_setTimeoutCallback cb, void *arg, int64 ms);

void ut_stitchText(void *outBuf, size_t *outBufLen, void *topBuf, size_t topBufLen,  void *botBuf, size_t botBufLen);
void ut_readIntoBuffer(char *filename, char *buffer, size_t *size);
void ut_sleepMS(int32 ms);
int64 ut_timestampMS(void);
size_t ut_clearAndDisplay(int fd, void *buf1, size_t size1);
void ut_merror(char *s);
