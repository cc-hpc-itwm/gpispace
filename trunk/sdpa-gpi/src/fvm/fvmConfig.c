#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>

#include "fvmConfig.h"

#define MAXLINE 100

static int getwords(char *line, char *words[], int maxwords)
{
  char *p = line;
  int nwords = 0;

  while(1)
    {
      while(isspace(*p))
	p++;

      if(*p == '\0')
	return nwords;

      words[nwords++] = p;

      while(!isspace(*p) && *p!= '\0')
	p++;

      if(*p == '\0')
	return nwords;

      if(nwords >= maxwords)
	return nwords;

      *p = '\0';
      p++;
    }
}

/* Internal routines to read config file to start FVM standalone*/

configFile_t readConfigFile(const char *filename, const int numOptions)
{

  char line[MAXLINE];
  char *words[2];
  char *p;

  FILE *ifp;

  int nw, na = 0;
  configFile_t *s = (configFile_t *)malloc(sizeof(configFile_t));
  s->msqfile = (char *) malloc(MAXLINE);
  s->shmemfile = (char *) malloc(MAXLINE);


  ifp = fopen(filename, "r");

  if (ifp == NULL) {
    fprintf(stderr, "Can't open FVM configuration file: %s!\n", filename);
    exit(1);
  }


  while(fgets(line, sizeof(line),ifp) !=NULL)
    {

      /* ignore comment lines (start with #) */
      if(*line == '#') 
	continue;

      nw = getwords(line, words, 2);
      if(nw < 2)
	continue;

      if( na >= numOptions)
	break;

      if(strcmp(words[0], "SHMSZ") == 0)
	{
	  s->shmemsize = atol(words[1]);
	  na++;
	}
      else if(strcmp(words[0], "FVMSZ") == 0)
	{
	  s->fvmsize = atoll(words[1]);
	  na++;
	}
      else if(strcmp(words[0], "MSQFILE") == 0)
	{
	  if(( p = strrchr(words[1],'\n')) != NULL)
	    *p = '\0';
	  strncpy(s->msqfile,words[1],MAXLINE);
	  na++;
	}
      else if(strcmp(words[0], "SHMFILE") == 0)
	{
	  if(( p = strrchr(words[1],'\n')) != NULL)
	    *p = '\0';
	  strncpy(s->shmemfile,words[1],MAXLINE);
	  na++;      
	}
      
    }
#ifndef NDEBUG
  printf("SHMSZ: %ld\nFVMSZ: %ld\nMSQFILE: %s\nSHMFILE: %s\n",
	 s->shmemsize,
	 s->fvmsize,
	 s->msqfile,
	 s->shmemfile);
#endif

  return *s;
}


