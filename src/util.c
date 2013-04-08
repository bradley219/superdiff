#ifndef _UTIL_SOURCE_
#define _UTIL_SOURCE_
#include "util.h"

int read_file_into_mem( char *filename, char **memory )
{
	int size;
	FILE *fp;

	fp = fopen( filename, "rb" );
	if( fp == NULL ) 
	{
		*memory = NULL;
		size = -1;
	}
	else 
	{
		fseek( fp, 0, SEEK_END );
		size = ftell( fp );
		fseek( fp, 0, SEEK_SET );
		*memory = (char *)malloc( sizeof(char) * (size + 1) );
		if( size != fread( *memory, sizeof(char), size, fp ) ) 
		{
			free(*memory);
			size = -2;
			*memory = NULL;
		}
		else {
			(*memory)[size] = '\0';
		}
		fclose( fp );
	}
	return size;
}
void substr( char *dest, char *src, int start, int end )
{
	char *d = dest;
	char *c = src;

	c += start;
	end -= start;
	while( end )
	{
		*d++ = *c++;
		end--;
	}
	*d = '\0';
	return;
}

#endif
