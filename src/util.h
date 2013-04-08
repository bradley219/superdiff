#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "globals.h"

int read_file_into_mem( char *filename, char **memory );
void substr( char *dest, char *src, int start, int end );

#endif
