%{
#include "glsl.tab.h"
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

[ \t\n] /* empty */
void { return VOID; }
int { return INT; }
float { return FLOAT; }
vec2 { return VEC2; }
vec3 { return VEC3; }
vec4 { return VEC4; }
mat2 { return MAT2; }
mat3 { return MAT3; }
mat4 { return MAT4; }
attribute { return ATTRIBUTE; }
uniform { return UNIFORM; }
in { return IN; }
layout { return LAYOUT; }
location { return LOCATION; }
out { return OUT; }
return { return RETURN; }
if { return IF; }
else { return ELSE; }
for { return FOR; }
struct { return STRUCT; }
version { return VERSION; }
precision { return PRECISION; }
lowp { return LOWP; }
mediump { return MEDIUMP; }
highp { return HIGHP; }
\# { return HASH; }
\{ { return LEFT_BRACE; }
\} { return RIGHT_BRACE; }
\( { return LEFT_PAREN; }
\) { return RIGHT_PAREN; }
\, { return COMMA; }
\; { return SEMICOLON; }
\= { return EQUAL; }
\+ { return PLUS; }
\- { return MINUS; }
\* { return STAR; }
\/ { return SLASH; }
\. { return DOT; }
\< { return LT; }
\> { return GT; }
\<\= { return LTE; }
\>\= { return GTE; }
\=\= { return EQ; }
\% { return MODULUS; }
\<\< { return LEFT_SHIFT; }
\>\> { return RIGHT_SHIFT; }
\& { return AMPERSAND; }
\^ { return CARET; }
\| { return PIPE; }

{identifier} { yylval.sval = __strdup(yytext); return IDENTIFIER; }
{digit}+ { yylval.ival = atoi(yytext); return INTCONST; }
{digit}+\.{digit}+ { yylval.sval = yytext; return FLOATCONST; }

%%

int yywrap() {
	return 1;
}