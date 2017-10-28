%{
#include "test-lang.tab.h"
#define register // Deprecated in c++11
#ifdef _WIN32
   #define __strdup _strdup
#else
   #define __strdup strdup
#endif
%}
%option yylineno
%%
\, { return COMMA; }
int { return INT; }
\{ { return LBRACE; }
\( { return LPAREN; }
\} { return RBRACE; }
\) { return RPAREN; }
void { return VOID; }
[_a-zA-Z][0-9_a-zA-Z]* { yylval.sval = __strdup(yytext); return identifier; }
[1-9][0-9]* { yylval.ival = atoi(yytext); return intConst; }
%%
int yywrap() { return 1; }
