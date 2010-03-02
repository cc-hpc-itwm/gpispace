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
#define REQUIRE_DEF(shift,field)                                              \
  {                                                                           \
    int mask = (1 << shift);                                                  \
                                                                              \
    if ((seen & mask) != mask)                                                \
      yyerror ("missing definition for " field);                              \
  }
#define WARN_DOUBLE_DEF(shift,field)                                          \
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
line : SHMSZ LONG    { cfg.shmemsize = $2; WARN_DOUBLE_DEF(0,"SHMSZ");   seen |= (1 << 0); }
     | FVMSZ LONG    { cfg.fvmsize   = $2; WARN_DOUBLE_DEF(1,"FVMSZ");   seen |= (1 << 1); }
     | MSQFILE WORD  { cfg.msqfile   = $2; WARN_DOUBLE_DEF(2,"MSQFILE"); seen |= (1 << 2); }
     | SHMFILE WORD  { cfg.shmemfile = $2; WARN_DOUBLE_DEF(3,"SHMFILE"); seen |= (1 << 3); }
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
  yyin = fopen(filename, "r");

  if (yyin == NULL)
    {
      perror ("open config file");
      exit(EXIT_FAILURE);
    }

  yyparse();

  fclose (yyin);

  REQUIRE_DEF(0, "SHMSZ");
  REQUIRE_DEF(1, "FVMSZ");
  REQUIRE_DEF(2, "MSQFILE");
  REQUIRE_DEF(3, "SHMFILE");

  return errcode;
}

#undef REQUIRE_DEF
#undef WARN_DOUBLE
