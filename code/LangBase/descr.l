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
string {return TOKEN_STRING; }
int {return TOKEN_INT; }
float {return TOKEN_FLOAT; }
start { return START; }
\{ { return LEFT_BRACE; }
\} { return RIGHT_BRACE; }
\( { return LEFT_PAREN; }
\) { return RIGHT_PAREN; }
\, { return COMMA; }
\: { return COLON; }
\< { return LBRACKET; }
\> { return RBRACKET; }
{identifier} { yylval.sval = __strdup(yytext); return IDENTIFIER; }
\"(\\.|[^"\\])*\" { yytext[strlen(yytext) - 1]=0; yylval.sval = __strdup(yytext+1); return STRING; }

%%

int yywrap() {
	return 1;
}