#include "common.h"

typedef struct Search Search;

Search *sh_createSearch(void *buffer, size_t bufferLen, char searchFor, size_t searchFrom);
void sh_freeSearch(Search *search);

//-1 if not found, else index of the `searchFor`
int sh_next(Search *search); 
