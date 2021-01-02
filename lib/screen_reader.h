#include "common.h"

typedef struct ScreenReader ScreenReader;

ScreenReader *sr_createScreenReader(void *buffer, size_t len, int linesPerDisplay);
void sr_freeScreenReader(ScreenReader *screenReader);

//returns -1 if done, 0 if success 
int32 sr_scroll(ScreenReader *screenReader, void *outBuffer, size_t *outBufferLen);
