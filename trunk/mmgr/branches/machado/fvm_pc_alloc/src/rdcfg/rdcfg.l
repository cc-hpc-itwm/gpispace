%{
#include <string.h>
#include "rdcfg.tab.h"
%}
%option noyywrap
%option nounput
%%
"SHMSZ"                                       return SHMSZ;
"FVMSZ"                                       return FVMSZ;
"MSQFILE"                                     return MSQFILE;
"SHMFILE"                                     return SHMFILE;
[0-9]+           yylval.num = atol(yytext);   return LONG;
[a-zA-Z0-9\/.-]+ yylval.str = strdup(yytext); return WORD;
[ \t\n=\"]                                    /* ignore whitespace */;
#.*                                           /* ignore comment lines */;
%%
