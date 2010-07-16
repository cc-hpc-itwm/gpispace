#ifndef _FVM_CONFIG_H_
#define _FVM_CONFIG_H_

typedef struct configFile {
  size_t shmemsize;
  size_t fvmsize;
  char * msqfile;
  char * shmemfile;
} configFile_t;


/* functions signatures */
configFile_t readConfigFile(const char *filename, const int numOptions);


#endif
