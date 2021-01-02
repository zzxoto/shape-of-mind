#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <time.h>
#include "common.h"
#include "utils.h"

size_t ut_clearAndDisplay(int fd, void *buf1, size_t size1)
{
  char buf2[] = { 27, 91, 72, 27, 91, 74 };
  size_t size2 = sizeof(buf2);

  //terminal CLS command + buffer
  char *str = (char *) malloc(size1 + size2);
  char *strPtr = str;

  memcpy(strPtr, buf2, size2);
  strPtr += size2;

  memcpy(strPtr, buf1, size1);

  //NOTE (Abhaya): assuming all buffer is flushed for now
  int32 ret = write(fd, str, size1 + size2);

  free(str);

  return ret;
}

void ut_readIntoBuffer(char *filename, char *buffer, size_t *size)
{
  int32 fd;
  if ((fd = open(filename, O_RDONLY)) == -1)
  {
    ut_merror("open");
    exit(1);
  }

  int32 nread;
  if ((nread = read(fd, buffer, *size)) == -1)
  {
    ut_merror("open");
    exit(1);
  }

  *size = nread;
  close(fd);
}

void ut_sleepMS(int32 ms)
{
  assert(ms >= 0);

  struct timeval timeout;
  timeout.tv_sec = ms / 1000;
  timeout.tv_usec = (ms % 1000) * 1000;

  select(100, NULL, NULL, NULL, &timeout);
}

int64 ut_timestampMS(void)
{
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);

  register int64 K = 1000000;

  int64 result = t.tv_sec * 1000 + (t.tv_nsec / K);
  if (t.tv_nsec % K >= K >> 1)
  {
    result++;
  }

  return result;
}

void ut_merror(char *s)
{
  printf("Error: %s (%d): %s\n", s, errno, strerror(errno));
}

/*
  topBuf:
  +++++++++++
  +++++++++++
  +++++++++++
  +++++++++++
  +++++++++++

  botBuf:
  ---------------------------
  ---------------------------
  ---------------------------
  ---------------------------
  ---------------------------

  outBuf:
  +++++++++++----------------
  +++++++++++----------------
  +++++++++++----------------
  +++++++++++----------------
  +++++++++++----------------
  ---------------------------
  ---------------------------

  repeat:
    Copy bot until ASCII_LF or bot ends to output buffer
    Copy top until ASCII_LF or top ends to output buffer
    if either top or bot had ASCII_LF, insert ASCII_LF at the end of output buffer
    if both top and bot ends, goto quit
    goto repeat

  quit:
    Note that carriage returns are ignored entirely in top/bot buffers.
    Also that if topBuf is a space and botBuf isn't a space at an intersecting
    buffer position, then content of botBuf is favored.

  NOTE (Abhaya): Went for text_canvas.c abstraction and thus this ain't needed
*/
void ut_stitchText(void *outBuf_, size_t *outBufLen, void *topBuf_, size_t topBufLen,  void *botBuf_, size_t botBufLen)
{
  size_t it = 0, ib = 0, i = 0;
  bool lfTop, lfBot;
  size_t len = *outBufLen;
  char *outBuf = (char *) outBuf_;
  char *topBuf = (char *) topBuf_;
  char *botBuf = (char *) botBuf_;

  memset(outBuf, 0, len);

  while ( (topBufLen > 0 || botBufLen > 0) && i < len)
  {
    lfTop = false;
    lfBot = false;

    it = i;
    ib = i;

    if (botBufLen > 0)
    {
      while (botBufLen > 0 && *botBuf != ASCII_LF && ib < len)
      {
        if (*botBuf == ASCII_CR)
        {
          botBuf++;
          botBufLen--;
        }
        else
        {
          outBuf[ib++] = *botBuf++;
          botBufLen--;
        }
      }
      if (botBufLen > 0 && *botBuf == ASCII_LF)
      {
        botBuf++;
        botBufLen--;
        lfBot = true;
      }
    }

    if (topBufLen > 0)
    {
      while (topBufLen > 0 && *topBuf != ASCII_LF && it < len)
      {
        if (*topBuf == ASCII_CR)
        {
          topBuf++;
          topBufLen--;
        }
        else if (*topBuf == ASCII_SPACE && outBuf[it] != 0)
        {
          topBuf++;
          topBufLen--;
          it++;
        }
        else
        {
          outBuf[it++] = *topBuf++;
          topBufLen--;
        }
      }
      if (topBufLen > 0 && *topBuf == ASCII_LF)
      {
        topBuf++;
        topBufLen--;
        lfTop = true;
      }
    }

    i = it > ib? it: ib;

    if ((lfTop || lfBot) && i < len)
    {
      outBuf[i++] = ASCII_CR;
      outBuf[i++] = ASCII_LF;
    }
  }

  if (i < len)
  {
    // since we have been doing i++ and it++ etc., above, we 
    // simply do *outBufLen = i rather than *outBufLen = i + 1;
    *outBufLen = i;
  }
}

