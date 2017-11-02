%{
#include "TestLang.tab.h"
#define register // Deprecated in c++11
#ifdef _WIN32
   #define __strdup _strdup
#else
   #define __strdup strdup
#endif
%}
%option yylineno
%%
\, { return COMMA_T; }
\= { return EQUAL_T; }
int { return INT_T; }
\{ { return LBRACE_T; }
\( { return LPAREN_T; }
\} { return RBRACE_T; }
\) { return RPAREN_T; }
\; { return SEMICOLON_T; }
void { return VOID_T; }
[_a-zA-Z][0-9_a-zA-Z]* { yylval.sval = __strdup(yytext); return identifier_T; }
[1-9][0-9]* { yylval.ival = atoi(yytext); return intConst_T; }
%%
int yywrap() { return 1; }
