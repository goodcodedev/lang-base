%{
#include <stdio.h>
#include "DescrNode.hpp"

extern FILE *yyin;
void yyerror(const char *s);
extern int yylex(void);
DescrNode *result;

template<typename T>
inline std::vector<T*>* push_node(void *vec, void *node) {
	std::vector<T*>* vec2 = reinterpret_cast<std::vector<T*>*>(vec);
	vec2->push_back(reinterpret_cast<T*>(node));
	return vec2;
}

template<typename T>
inline T* re(void *node) {
	return reinterpret_cast<T*>(node);
}

extern int yylineno;

%}
%union {
	int ival;
	double fval;
	char *sval;
	void *ast;
	void *vector;
	int enm;
}

%token <sval> IDENTIFIER
%token TOKEN
%token ENUM
%token AST
%token LIST
%token LEFT_BRACE
%token RIGHT_BRACE
%token LEFT_PAREN
%token RIGHT_PAREN
%token COMMA
%token COLON
%token <sval> STRING

%type <ast> source token enum enum_decl ast ast_def ast_part list
%type <vector> nodes enum_decls ast_defs ast_parts

%%
source: nodes { result = new SourceNode(reinterpret_cast<std::vector<DescrNode*>*>($1)); }
    ;

nodes: /* empty */ {
        $$ = new std::vector<DescrNode*>;
    }
    | nodes token { $$ = push_node<DescrNode>($1, $2); }
    | nodes enum { $$ = push_node<DescrNode>($1, $2); }
    | nodes ast { $$ = push_node<DescrNode>($1, $2); }
    | nodes list { $$ = push_node<DescrNode>($1, $2); }
    ;

token: TOKEN IDENTIFIER STRING {
    $$ = new TokenNode($2, $3);
}
enum: ENUM IDENTIFIER LEFT_BRACE enum_decls RIGHT_BRACE;
enum_decls: /* empty */ {
        $$ = new std::vector<EnumDecl*>;
    }
    | enum_decls enum_decl { $$ = push_node<EnumDecl>($1, $2); }
    | enum_decls COMMA enum_decl { $$ = push_node<EnumDecl>($1, $3); }
    ;
enum_decl: IDENTIFIER STRING { $$ = new EnumDecl($1, $2); }
ast: AST IDENTIFIER LEFT_PAREN ast_def RIGHT_PAREN { 
        $$ = new AstNode($2, new std::vector<AstDef*>({reinterpret_cast<AstDef*>($4)}));
     }
    | AST IDENTIFIER COLON IDENTIFIER LEFT_PAREN ast_defs RIGHT_PAREN {
        $$ = new AstNode($2, $4, $6);
    }
    ;
ast_defs: /* empty */ {
        $$ = new std::vector<AstDef*>;
    }
    | ast_defs ast_def { $$ = push_node<AstDef>($1, $2); }
    | ast_defs COMMA ast_def { $$ = push_node<AstDef>($1, $3); }
    ;
ast_def: IDENTIFIER ast_parts { $$ = new AstDef(reinterpret_cast<std::vector<AstPart*>*>($2)); };
ast_parts: /* empty */ {
        $$ = new std::vector<AstPart*>;
    }
    | ast_def ast_part { $$ = push_node<AstPart>($1, $2); }
    ;

ast_part: IDENTIFIER { $$ = new AstPart($1); };

list: LIST IDENTIFIER IDENTIFIER IDENTIFIER { $$ = new ListNode($2, $3, $4); };

%%

void yyerror(const char *s) {
	printf("Parse error on line %d: %s", yylineno, s);
}