#include <assert.h>
#include <unistd.h>
#include "text_canvas.h"

#define IS_PRINTABLE(a) ((a) >= 32 && (a) <= 126)

typedef struct TextCanvas
{
  int32 cols;
  int32 rows;
  char *buffer;
  size_t bufferLen;
  bool shouldIgnoreNonPrintable;
} TextCanvas;

TextCanvas *tc_createTextCanvas(int32 cols, int32 rows, bool shouldIgnoreNonPrintable)
{
  assert(cols > 0); assert(rows > 0);

  size_t bufferLen = (cols + 1) * rows;
  char *buffer     = (char *) malloc(bufferLen);

  TextCanvas *tc = (TextCanvas *) malloc(sizeof(TextCanvas));
  tc->cols = cols;
  tc->rows = rows;
  tc->buffer    = buffer;
  tc->bufferLen = bufferLen;
  tc->shouldIgnoreNonPrintable = shouldIgnoreNonPrintable;

  tc_clearTextCanvas(tc);

  return tc;
}

void tc_clearTextCanvas(TextCanvas *canvas)
{
  int32 cols2      = canvas->cols + 1;
  char *buffer     = (char *) canvas->buffer;
  size_t bufferLen = canvas->bufferLen;

  for(int32 i = 0; i < bufferLen; i++)
  {
    if ((i + 1) % cols2 == 0)
    {
      buffer[i] = ASCII_LF;
    }
    else
    {
      buffer[i] = ASCII_SPACE;
    }
  }
}

void tc_blit(TextCanvas *canvas, void *src_, size_t srcLen, Rect *rect)
{
  assert(rect->x < canvas->cols && rect->x >= 0);
  assert(rect->y < canvas->rows && rect->y >= 0);
  assert(rect->cols > 0);
  assert(rect->rows > 0);

  int32 rowA = rect->y + 1;
  int32 rowB = rowA + rect->rows - 1;
  rowB = MIN(rowB, canvas->rows);

  int32 colA = rect->x + 1;
  int32 colB = colA + rect->cols - 1;
  colB = MIN(colB, canvas->cols);

  char *src  = (char *) src_;
  char *dest = (char *) canvas->buffer;

  bool isNonPrintableGood = !canvas->shouldIgnoreNonPrintable;

  for (int32 r = rowA, i_, i; r <= rowB && srcLen > 0; r++)
  {
    i_ = (canvas->cols + 1) * (r - 1);

    for (int32 c = colA; c <= colB && *src != ASCII_LF && srcLen > 0; c++)
    {
      if (isNonPrintableGood || IS_PRINTABLE(*src))
      {
        i = i_ + c - 1;
        dest[i] = *src++;
        srcLen--;
      }
      else
      {
        src++;
        srcLen--;
        c--;
      }
    }

    //post inner for loop check
    if (srcLen <= 0)
    {
      break;
    }
    if (*src == ASCII_LF)
    {
      src++;
      srcLen--;
      continue;
    }
    else
    {
      //consume src until next new line
      while (*src != ASCII_LF && srcLen > 0)
      {
        srcLen--;
        src++;
      }
      if (srcLen > 0)
      {
        src++;
        srcLen--;
      }
    }
  }
}

void tc_readIntoBuffer(TextCanvas *canvas, void *buffer, size_t *len)
{
  assert(len >= 0);

  char *dest = (char *) buffer;
  size_t destLen = *len;

  char *src  = (char *) canvas->buffer;
  size_t srcLen = canvas->bufferLen;

  size_t count = 0;
  while(srcLen-- > 0 && destLen-- > 0 && ++count)
  {
    *dest++ = *src++;
  }

  *len = count;
}

void tc_freeTextCanvas(TextCanvas *canvas)
{
  free(canvas->buffer);
  free(canvas);
}

void tc_printCanvas(TextCanvas *canvas)
{
  size_t srcLen = canvas->bufferLen;
  char *src     = (char *) canvas->buffer;

  size_t destLen = srcLen * 3;
  char *dest     = (char *) malloc(destLen);
  memset(dest, 0, destLen);
  char *destHead = dest;

  for (int32 i = 0; i < srcLen; i++)
  {
    if (i == 0 || (*(src - 1) == ASCII_LF))
    {
      *dest++ = '|';
    }

    char c  = *src++;
    *dest++ = c;

    if (c != ASCII_LF)
    {
      *dest++ = '|';
    }
  }

  write(1, destHead, destLen);

  free(destHead);
}

#if 0
int main(void)
{
  //(0, 0)  (6, 0)
  //(0, 3)  (6, 3)
  TextCanvas *canvas;
  {
    int32 cols = 3;
    int32 rows = 3;

    canvas = tc_createTextCanvas(cols, rows, true);
  }

  char buffer[] = { 'A', 'B', 'K', 'C', ASCII_LF, 'D', 'E', 'F', ASCII_LF };
  size_t bufferLen = sizeof(buffer);

  for (int r = 0; r < canvas->rows; r++)
  {
    for (int c = 0; c < canvas->cols; c++)
    {
      Rect rect;
      rect.x = c;
      rect.y = r; 
      rect.cols = 8; 
      rect.rows = 8; 

      tc_blit(canvas, (void *) buffer, bufferLen, &rect);
      tc_printCanvas(canvas);
      tc_clearTextCanvas(canvas);
      sleepMS(500);
    }
  }

  return 0;
}
#endif
