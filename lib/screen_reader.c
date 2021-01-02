#include <unistd.h>
#include "screen_reader.h"
#include "search.h"
#include "utils.h"

typedef struct ScreenReader 
{
  char *buffer;
  size_t bufferLen;
  size_t bufferPos;

  int32 linesPerDisplay;
  bool isDone;
} ScreenReader;

ScreenReader *sr_createScreenReader(void *buffer, size_t len, int32 linesPerDisplay)
{
  assert(linesPerDisplay >= 1);
  assert(buffer);

  ScreenReader *sr = (ScreenReader *) malloc(sizeof (ScreenReader));
  sr->buffer = (char *) buffer;
  sr->bufferPos = 0;
  sr->bufferLen = len;
  sr->linesPerDisplay = linesPerDisplay;
  sr->isDone = false;

  return sr;
}

void sr_freeScreenReader(ScreenReader *sr)
{
  free(sr);
}

/*
   a
   b
   c
   d
   e
   f

   abc, bcd, cde, def, ef, f, -1
 */

int32 sr_scroll(ScreenReader *sr, void *outBuffer_, size_t *outBufferLen)
{
  if (sr->isDone)
  {
    return -1;
  }

  char *windowStart = sr->buffer + sr->bufferPos;
  size_t windowSize;

  int nextLFPos;
  {
    Search *search = sh_createSearch(sr->buffer, sr->bufferLen, ASCII_LF, sr->bufferPos);
    nextLFPos = sh_next(search);
    sh_freeSearch(search);
  }

  if (nextLFPos < 0 || (nextLFPos + 1 >= sr->bufferLen))
  {
      windowSize = sr->bufferLen - sr->bufferPos;
      sr->bufferPos = sr->bufferLen - 1;
      sr->isDone = true;
  }
  else
  {
    int n = -1;
    {
      Search *search = sh_createSearch(sr->buffer, sr->bufferLen, ASCII_LF, sr->bufferPos);

      int count = sr->linesPerDisplay;
      while(count--)
      {
        n = sh_next(search);
      }
      sh_freeSearch(search);
    }

    if (n > 0)
    {
      windowSize = n - sr->bufferPos;
    }
    else
    {
      windowSize = sr->bufferLen - sr->bufferPos;
    }

    sr->bufferPos = nextLFPos + 1;
  }

  char *outBuffer = (char *) outBuffer_;
  size_t until = *outBufferLen < windowSize? *outBufferLen: windowSize;
  *outBufferLen = until;
  while (until--)
  {
    *outBuffer++ = *windowStart++;
  }

  return 0;
}
