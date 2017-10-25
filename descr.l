%{
#include "descr.tab.h"
#define register      // Deprecated in C++11.
#ifdef _WIN32
	#define __strdup _strdup
#else
	#define __strdup strdup
#endif
%}
%option yylineno

digit [0-9]
nondigit [_a-zA-Z]
identifier {nondigit}({nondigit}|{digit})*

%x COMMENT
%%

<INITIAL>"/*" BEGIN(COMMENT);
<COMMENT>[^*] { }
<COMMENT>"*"+[^/] { }
<COMMENT>"*/" BEGIN(INITIAL);

token { return TOKEN; }
enum { return ENUM; }
ast { return AST; }
list { return LIST; }
\{ { return LEFT_BRACE; }
\} { return RIGHT_BRACE; }
\, { return COMMA; }
\: { return COLON; }
{identifier} { yylval.sval = __strdup(yytext); return IDENTIFIER; }
\"(\\.|[^"\\])*\" { yylval.sval = __strdup(yytext); return STRING; }

%%

int yywrap() {
	return 1;
}