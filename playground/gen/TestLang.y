%{
#include <stdio.h>
#include "TestLang.hpp"
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
%token COMMA_T
%token EQUAL_T
%token INT_T
%token LBRACE_T
%token LPAREN_T
%token RBRACE_T
%token RPAREN_T
%token SEMICOLON_T
%token VOID_T
%token <sval> identifier_T
%token <ival> intConst_T
%type <ival> Type 
%type <ptr> start ControlStruct Function IntExpr expr argExprs statements 
%%
start: Function { result = reinterpret_cast<Function*>($1);$$ = result; }
    ;
ControlStruct: expr { $$ = new If(reinterpret_cast<Expression*>($1)); }
    ;
Function: Type identifier_T LPAREN_T argExprs RPAREN_T LBRACE_T statements RBRACE_T { $$ = new Function(static_cast<Type>($1), $2, reinterpret_cast<std::vector<Expression*>*>($4), reinterpret_cast<std::vector<Statement*>*>($7)); }
    ;
IntExpr: intConst_T { $$ = new IntExpr($1); }
    ;
expr: IntExpr { $$ = $1; }
    | identifier_T { $$ = new IdExpr($1); }
    ;
argExprs: { $$ = new std::vector<Expression*>; }
    | argExprs expr { std::vector<Expression*>* vec = reinterpret_cast<std::vector<Expression*>*>($1);vec->push_back($2);$$ = vec; }
    | argExprs COMMA_T expr { std::vector<Expression*>* vec = reinterpret_cast<std::vector<Expression*>*>($1);vec->push_back($3);$$ = vec; }
    ;
statements: { $$ = new std::vector<Statement*>; }
    | statements identifier_T EQUAL_T expr SEMICOLON_T { std::vector<Statement*>* vec = reinterpret_cast<std::vector<Statement*>*>($1);vec->push_back(new Assign($2, reinterpret_cast<Expression*>($4)));$$ = vec; }
    | statements ControlStruct { std::vector<Statement*>* vec = reinterpret_cast<std::vector<Statement*>*>($1);vec->push_back($2);$$ = vec; }
    ;
Type: VOID_T { $$ = VOID; }
    | INT_T { $$ = INT; }
    ;

%%
void yyerror(const char *s) {
    printf("Parse error on line %d: %s", yylineno, s);
}
