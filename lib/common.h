#ifndef _MY_COMMON_H_
#define _MY_COMMON_H_

#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

typedef unsigned char uchar;
typedef unsigned int uint32;
typedef int int32;
typedef unsigned long int uint64;
typedef long int int64;

#define internal static
#define ASCII_LF 10
#define ASCII_CR 13
#define ASCII_SPACE 32

#define MIN(a, b) ((a) < (b)? (a): (b))
#define MAX(a, b) ((a) > (b)? (a): (b))

#endif

