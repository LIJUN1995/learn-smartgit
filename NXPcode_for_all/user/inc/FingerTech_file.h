#include "main.h"


void *fopen_M7_cfp( const char *path, char mode );
int fclose_M7_cfp( void *stream );
int fread_M7_cfp( void *buffer, int size, int count, void *stream );
int fwrite_M7_cfp( const void *buffer, int size, int count, void *stream );
int fsync_M7_cfp( void *stream );
int fmkdir_M7_cfp( void *path );
void sprintf_M7_cfp( char *buf, const char *format, ... );


