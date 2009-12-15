#ifndef _FVM_CONFIG_H_
#define _FVM_CONFIG_H_

#include <stdlib.h>

typedef struct configFile
{
  size_t shmemsize;
  size_t fvmsize;
  char *msqfile;
  char *shmemfile;
} configFile_t;

configFile_t cfg;

/* ret == 0 ==> success, that is all values are read */
int readConfigFile (const char *filename);

#include <fvmConfig.tab.h>

#endif
