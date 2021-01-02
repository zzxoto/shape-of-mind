#include "search.h"

typedef struct Search
{
  char *buffer;
  size_t bufferLen;
  size_t bufferPos;

  char searchFor;
  bool isDone;
} Search;

Search *sh_createSearch(void *buffer, size_t bufferLen, char searchFor, size_t searchFrom)
{
  Search *search    = (Search *) malloc(sizeof(Search));
  search->buffer    = (char *) buffer;
  search->bufferLen = bufferLen;
  search->bufferPos = searchFrom;
  search->searchFor = searchFor;
  search->isDone    = bufferLen <= 0 || searchFrom >= bufferLen - 1;

  return search;
}

void sh_freeSearch(Search *search)
{
  free(search);
}

int sh_next(Search *search)
{
  if (search->isDone)
  {
    return -1;
  }

  char *ptr = search->buffer + search->bufferPos;
  char target = search->searchFor;

  size_t i = search->bufferPos;
  size_t count = search->bufferLen;

  while(i < count)
  {
    if (*ptr++ == target)
    {
      search->bufferPos = i + 1;
      search->isDone = search->bufferPos >= search->bufferLen;
      return i;
    }
    i++;
  }

  search->isDone = true;
  return -1;
}
