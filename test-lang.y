%{
#include <stdio.h>
Function* result;
extern FILE *yyin;
void yyerror(const char *s);
extern int yylex(void);
extern int yylineno;
%}
%union {
   void *ptr;
    char *sval;
    int ival;
}
%token COMMA
%token INT
%token LBRACE
%token LPAREN
%token RBRACE
%token RPAREN
%token VOID
%token <sval> identifier
%token <ival> intConst
%type <ival> Type 
%type <ptr> start Function IntExpr expr argExprs 
%%
start: Function { result = reinterpret_cast<Function*>($1);$$ = result; }
    ;
Function: Type identifier LPAREN argExprs RPAREN LBRACE RBRACE { $$ = new Function(static_cast<Type>($1), $2, reinterpret_cast<std::vector<Expression*>*>($4)); }
    ;
IntExpr: intConst { $$ = new IntExpr($1); }
    ;
expr: IntExpr { $$ = $1; }
    | identifier { $$ = new IdExpr($1); }
    ;
argExprs: { $$ = new std::vector<Expression*>; }
    | argExprs expr { std::vector<Expression*>* vec = reinterpret_cast<std::vector<Expression*>*>($1);vec->push_back(reinterpret_cast<Expression*>($2));$$ = vec; }
    | argExprs COMMA expr { std::vector<Expression*>* vec = reinterpret_cast<std::vector<Expression*>*>($1);vec->push_back(reinterpret_cast<Expression*>($3));$$ = vec; }
    ;
Type: VOID { $$ = VOID; }
    | INT { $$ = INT; }
    ;
