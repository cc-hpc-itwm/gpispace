%{
#include <string.h>
#include "fvmConfig.tab.h"
%}
%option noyywrap
%option nounput
%option noinput
%%
"SHMSZ"                                                     return SHMSZ;
"FVMSZ"                                                     return FVMSZ;
"MSQFILE"                                                   return MSQFILE;
"SHMFILE"                                                   return SHMFILE;
[0-9]+          yylval.num = atol(yytext);                  return LONG;
[^ \t\n=\"\']+  yylval.str = strdup(yytext);                return WORD;
\"[^\t\n=\"]+\" yylval.str = strndup(yytext + 1, yyleng-2); return WORD;
\'[^\t\n=\']+\' yylval.str = strndup(yytext + 1, yyleng-2); return WORD;
[ \t\n=]        /* ignore whitespace */                     ;
#.*             /* ignore comments */                       ;
%%
