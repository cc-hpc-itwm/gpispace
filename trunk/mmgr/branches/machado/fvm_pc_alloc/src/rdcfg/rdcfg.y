%{
#include <stdio.h>
#include <stdlib.h>
static void yyerror(char *);
static void yywarn(char *);
int yylex (void);
FILE *yyin;
#include <fvmConfig.h>
static int seen = 0;
static int errcode = 0;
#define CHECK_DEF(shift,field)                                                \
  {                                                                           \
    int mask = (1 << shift);                                                  \
                                                                              \
    if ((seen & mask) != mask)                                                \
      yyerror ("missing definition for " field);                              \
  }
#define CHECK_DOUBLE(shift,field)                                             \
  {                                                                           \
    int mask = (1 << shift);                                                  \
                                                                              \
    if ((seen & mask) == mask)                                                \
      yywarn ("overwritten definition for " field);                           \
  }
%}
%union
{
  unsigned long num;
  char * str;
}
%token SHMSZ FVMSZ MSQFILE SHMFILE
%token <num> LONG
%token <str> WORD
%%
cfg  : line
     | line cfg
     ;
line : SHMSZ LONG   { cfg.shmemsize = $2; CHECK_DOUBLE(0,"SHMSZ");   seen |= (1 << 0); }
     | FVMSZ LONG   { cfg.fvmsize   = $2; CHECK_DOUBLE(1,"FVMSZ");   seen |= (1 << 1); }
     | MSQFILE WORD { cfg.msqfile   = $2; CHECK_DOUBLE(2,"MSQFILE"); seen |= (1 << 2); }
     | SHMFILE WORD { cfg.shmemfile = $2; CHECK_DOUBLE(3,"SHMFILE"); seen |= (1 << 3); }
     ;
%%
static void yymsg (char * pref, char * msg)
{
  fprintf (stderr, "%s parsing config file: %s\n", pref, msg);
}

static void yyerror (char *msg)
{
  yymsg ("error", msg); errcode = -1;
}

static void yywarn (char *msg)
{
  yymsg ("warning when", msg);
}

int readConfigFile (const char *filename)
{
  FILE * f = fopen(filename, "r");

  if (f == NULL)
    {
      perror ("open config file");
      exit(EXIT_FAILURE);
    }

  yyin = f;

  yyparse();

  CHECK_DEF(0, "SHMSZ");
  CHECK_DEF(1, "FVMSZ");
  CHECK_DEF(2, "MSQFILE");
  CHECK_DEF(3, "SHMFILE");

  return (errcode < 0) ? errcode : seen;
}

#undef CHECK_DEF
