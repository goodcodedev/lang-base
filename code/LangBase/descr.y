%{
#include <stdio.h>
#include "../DescrNode.hpp"

using namespace LangBase;

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
%token LBRACKET
%token RBRACKET
%token COMMA
%token COLON
%token START
%token <sval> STRING
%token TOKEN_STRING
%token TOKEN_INT
%token TOKEN_FLOAT

%type <ast> source token_decl enum_def enum_decl ast ast_def ast_part list type_decl start list_def
%type <vector> nodes enum_decls ast_defs ast_parts list_defs
%type <enm> tokenType

%%
source: nodes { result = new SourceNode(reinterpret_cast<std::vector<DescrNode*>*>($1)); }
    ;

nodes: /* empty */ {
        $$ = new std::vector<DescrNode*>;
    }
    | nodes token_decl { $$ = push_node<DescrNode>($1, $2); }
    | nodes enum_def { $$ = push_node<DescrNode>($1, $2); }
    | nodes ast { $$ = push_node<DescrNode>($1, $2); }
    | nodes list { $$ = push_node<DescrNode>($1, $2); }
    | nodes start { $$ = push_node<DescrNode>($1, $2); }
    ;
start: START IDENTIFIER { $$ = new StartNode($2); }
    ;

tokenType: TOKEN_STRING { $$ = TSTRING; }
        | TOKEN_INT { $$ = TINT; }
        | TOKEN_FLOAT { $$ = TFLOAT; }
        ;

token_decl: TOKEN IDENTIFIER STRING {
        $$ = new TokenNode($2, $3);
    }
    | TOKEN IDENTIFIER COLON tokenType STRING {
        $$ = new TokenNode(static_cast<TokenType>($4), $2, $5);
    }
    ;
enum_def: ENUM type_decl LEFT_BRACE enum_decls RIGHT_BRACE { 
      $$ = new EnumNode(re<TypeDecl>($2), reinterpret_cast<std::vector<EnumDeclNode*>*>($4)); 
    };
enum_decls: /* empty */ {
        $$ = new std::vector<EnumDeclNode*>;
    }
    | enum_decls enum_decl { $$ = push_node<EnumDeclNode>($1, $2); }
    | enum_decls COMMA enum_decl { $$ = push_node<EnumDeclNode>($1, $3); }
    ;
enum_decl: IDENTIFIER STRING { $$ = new EnumDeclNode($1, $2); }
type_decl: IDENTIFIER { $$ = new TypeDecl($1); }
            | IDENTIFIER COLON IDENTIFIER { $$ = new TypeDecl($3, $1); }
            ;
ast: AST type_decl LEFT_PAREN ast_parts RIGHT_PAREN { 
        $$ = new AstNode(re<TypeDecl>($2), new std::vector<AstDef*>({new AstDef("", reinterpret_cast<std::vector<AstPart*>*>($4))}));
     }
    | AST type_decl LEFT_BRACE ast_defs RIGHT_BRACE {
        $$ = new AstNode(re<TypeDecl>($2), reinterpret_cast<std::vector<AstDef*>*>($4));
    }
    ;
ast_defs: /* empty */ {
        $$ = new std::vector<AstDef*>;
    }
    | ast_defs ast_def { $$ = push_node<AstDef>($1, $2); }
    | ast_defs COMMA ast_def { $$ = push_node<AstDef>($1, $3); }
    ;
ast_def: IDENTIFIER { $$ = new AstDef($1, new std::vector<AstPart*>); }
        | IDENTIFIER LEFT_PAREN ast_parts RIGHT_PAREN { $$ = new AstDef($1, reinterpret_cast<std::vector<AstPart*>*>($3)); }
        | LEFT_PAREN ast_parts RIGHT_PAREN { $$ = new AstDef("", reinterpret_cast<std::vector<AstPart*>*>($2)); }
        ;
ast_parts: /* empty */ {
        $$ = new std::vector<AstPart*>;
    }
    | ast_parts ast_part { $$ = push_node<AstPart>($1, re<AstPart>($2)); }
    ;

ast_part: IDENTIFIER { $$ = new AstPart($1); }
          | IDENTIFIER COLON IDENTIFIER { $$ = new AstPart($3, $1); }
          ;

list: LIST type_decl IDENTIFIER IDENTIFIER { $$ = new ListNode(re<TypeDecl>($2), $3, $4); }
    | LIST type_decl LEFT_BRACE list_defs RIGHT_BRACE { 
        $$ = new ListNode(re<TypeDecl>($2), reinterpret_cast<std::vector<ListDef*>*>($4)); 
    }
    ;

list_defs: /* empty */ {
        $$ = new std::vector<ListDef*>;
    }
    | list_defs list_def { $$ = push_node<ListDef>($1, $2); }
    | list_defs COMMA list_def { $$ = push_node<ListDef>($1, $3); }
    ;
list_def: IDENTIFIER { $$ = new ListDef($1, new std::vector<AstPart*>); }
        | IDENTIFIER IDENTIFIER { $$ = new ListDef($1, new std::vector<AstPart*>, $2); }
        | IDENTIFIER LEFT_PAREN ast_parts RIGHT_PAREN { 
            $$ = new ListDef($1, reinterpret_cast<std::vector<AstPart*>*>($3));
        }
        | IDENTIFIER IDENTIFIER LEFT_PAREN ast_parts RIGHT_PAREN {
            $$ = new ListDef($1, $2, reinterpret_cast<std::vector<AstPart*>*>($4));
        }
        | IDENTIFIER LEFT_PAREN ast_parts RIGHT_PAREN IDENTIFIER { 
            $$ = new ListDef($1, reinterpret_cast<std::vector<AstPart*>*>($3), $5); 
        }
        ;

%%

void yyerror(const char *s) {
	printf("Parse error on line %d: %s", yylineno, s);
}