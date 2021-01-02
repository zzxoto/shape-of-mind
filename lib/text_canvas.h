#include "common.h"

typedef struct TextCanvas TextCanvas;

typedef struct 
{
  int32 x;
  int32 y;
  int32 cols;
  int32 rows;
} Rect;

TextCanvas *tc_createTextCanvas(int32 cols, int32 rows, bool shouldIgnoreNonPrintable);
void tc_clearTextCanvas(TextCanvas *canvas);
void tc_blit(TextCanvas *canvas, void *buffer, size_t len, Rect *r);
void tc_readIntoBuffer(TextCanvas *canvas, void *buffer, size_t *len);

//for debugging
void tc_printCanvas(TextCanvas *canvas);

void tc_freeTextCanvas(TextCanvas *canvas);
