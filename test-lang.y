%{
#include <stdio.h>
extern FILE *yyin;
void yyerror(const char *s);
extern int yylex(void);
extern int yylineno;
%}
%union {
   void *ptr;
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
%type <ptr> Function IntExpr expr argExprs 
%%
Type:  VOID { $$ = VOID; }
    | INT { $$ = INT; }
    ;
Function:  Type identifier LPAREN argExprs RPAREN LBRACE RBRACE { $$ = new Function(static_cast<Type>($1), $2, reinterpret_cast<std::vector<Expression*>>($4)); }
    ;
IntExpr:  intConst { $$ = new IntExpr($1); }
    ;
expr:  IntExpr { $$ = $1; }
    | identifier { $$ = new IdExpr($1); }
    | identifier intConst { $$ = new Expression($1, $2); }
    ;
argExprs:  { $$ = new std::vector<Expression*>; }
    | argExprs expr { Expression* vec = reinterpret_cast<Expression*>($1);vec->push_back(reinterpret_cast<Expression*>($2));$$ = vec; }
    | argExprs COMMA expr { Expression* vec = reinterpret_cast<Expression*>($1);vec->push_back(reinterpret_cast<Expression*>($3));$$ = vec; }
    ;
