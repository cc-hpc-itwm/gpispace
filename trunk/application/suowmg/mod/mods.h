
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#include "tools.h"


typedef struct {
  FILE *fp;
  long long int currentPos;
} modsFILE;

modsFILE *mods_fopen(char *traceFile,char *permission);
void mods_fclose(modsFILE *mFILE);
void mods_fread(modsFILE *mFILE, float *buf, long long int pos, long long int count);

