%{
#include <stdio.h>
#include "AstNode.hpp"

extern FILE *yyin;
void yyerror(const char *s);
extern int yylex(void);
inline Type itot(int i) {
	return static_cast<Type>(i);
}
AstNode *result;

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

%token <ival> INTCONST
%token <sval> FLOATCONST
%token <sval> IDENTIFIER
%token VOID
%token INT
%token FLOAT
%token VEC2
%token VEC3
%token VEC4
%token MAT2
%token MAT3
%token MAT4
%token LEFT_BRACE
%token RIGHT_BRACE
%token LEFT_PAREN
%token RIGHT_PAREN
%token COMMA
%token SEMICOLON
%token EQUAL
%token ATTRIBUTE
%token UNIFORM
%token LAYOUT
%token LOCATION
%token IN
%token OUT
%token PLUS
%token MINUS
%token STAR
%token SLASH
%token MODULUS
%token LEFT_SHIFT
%token RIGHT_SHIFT
%token AMPERSAND
%token CARET
%token PIPE
%token DOT
%token RETURN
%token IF
%token ELSE
%token FOR
%token LT
%token GT
%token LTE
%token GTE
%token EQ
%token STRUCT
%token HASH
%token VERSION
%token PRECISION
%token LOWP
%token MEDIUMP
%token HIGHP
%type <ast> source
%type <ast> function
%type <ast> attribute_decl
%type <ast> in_decl
%type <ast> out_decl
%type <ast> uniform_decl
%type <ast> global_decl
%type <enm> type
%type <enm> type_constructable
%type <vector> statements
%type <ast> statement
%type <ast> control_struct
%type <ast> expression
%type <ast> block
%type <vector> nodes
%type <vector> arg_list
%type <ast> arg_decl
%type <vector> arg_decl_list
%type <enm> comp_op
%type <enm> assign_op
%type <ast> struct_decl
%type <vector> struct_members
%type <ast> version_decl
%type <ast> precision_decl
%type <enm> precision_qualifier

%%
source: nodes { result = new Source(reinterpret_cast<std::vector<AstNode*>*>($1)); }
	;

nodes:		/* empty */ {
				$$ = new std::vector<AstNode*>;
			}
			| nodes function		{ $$ = push_node<AstNode>($1, $2); }
			| nodes attribute_decl	{ $$ = push_node<AstNode>($1, $2); }
			| nodes in_decl			{ $$ = push_node<AstNode>($1, $2); }
			| nodes out_decl		{ $$ = push_node<AstNode>($1, $2); }
			| nodes uniform_decl	{ $$ = push_node<AstNode>($1, $2); }
			| nodes global_decl		{ $$ = push_node<AstNode>($1, $2); }
			| nodes struct_decl 	{ $$ = push_node<AstNode>($1, $2); }
			| nodes version_decl	{ $$ = push_node<AstNode>($1, $2); }
			| nodes precision_decl 	{ $$ = push_node<AstNode>($1, $2); }
			;

precision_decl: PRECISION precision_qualifier FLOAT { $$ = new Precision(static_cast<PrecisionQualifier>($2), Float); }
				| PRECISION precision_qualifier INT { $$ = new Precision(static_cast<PrecisionQualifier>($2), Int); }
				;

precision_qualifier: LOWP { $$ = Lowp; }
					| MEDIUMP { $$ = Mediump; }
					| HIGHP { $$ = Highp; }
					;

version_decl: HASH VERSION INTCONST { $$ = new Version($3); };
attribute_decl: ATTRIBUTE type IDENTIFIER SEMICOLON	{ $$ = new Attribute(itot($2), $3); };
in_decl: IN type IDENTIFIER SEMICOLON				{ $$ = new In(itot($2), $3); }
		| LAYOUT LEFT_PAREN LOCATION EQUAL INTCONST RIGHT_PAREN IN type IDENTIFIER SEMICOLON {
			$$ = new In(itot($8), $9, $5);
		}
		;
out_decl: OUT type IDENTIFIER SEMICOLON				{ $$ = new Out(itot($2), $3); };
uniform_decl: UNIFORM type IDENTIFIER SEMICOLON		{ $$ = new Uniform(itot($2), $3); };
global_decl: type IDENTIFIER EQUAL expression SEMICOLON	{ $$ = new TypedAssignment(itot($1), $2, re<Expression>($4)); };

struct_decl: STRUCT IDENTIFIER LEFT_BRACE struct_members RIGHT_BRACE SEMICOLON {
				$$ = new StructDecl(reinterpret_cast<std::vector<StructMember*>*>($4), $2);
			}
			| STRUCT IDENTIFIER LEFT_BRACE struct_members RIGHT_BRACE IDENTIFIER SEMICOLON {
				$$ = new StructDecl(reinterpret_cast<std::vector<StructMember*>*>($4), $2, $6);
			}
			;

struct_members: /* empty */ {
				$$ = new std::vector<StructMember*>;
			}
			| struct_members type IDENTIFIER SEMICOLON {
				$$ = push_node<StructMember>($1, new StructMember(itot($2), $3));
			}
			;

function:	type IDENTIFIER LEFT_PAREN arg_decl_list RIGHT_PAREN block {
				$$ = new Function(itot($1), $2, re<std::vector<ArgDecl*>>($4), re<Block>($6));
			}
			;

block:	LEFT_BRACE statements RIGHT_BRACE {
			$$ = new Block(reinterpret_cast<std::vector<Statement*>*>($2));
		}
		;

statements: /* empty */ {
				$$ = new std::vector<Statement*>;
			}
			| statements statement SEMICOLON {
				$$ = push_node<Statement>($1, $2);
			}
			| statements control_struct {
				$$ = push_node<Statement>($1, $2);
			}
			;

statement:	type IDENTIFIER EQUAL expression {
				$$ = new TypedAssignment(itot($1), $2, re<Expression>($4));
			}
			| IDENTIFIER EQUAL expression {
				$$ = new Assignment($1, re<Expression>($3));
			}
			| IDENTIFIER assign_op expression {
				$$ = new OpAssignment(static_cast<AssignOp>($2), $1, re<Expression>($3));
			}
			| IDENTIFIER LEFT_PAREN arg_list RIGHT_PAREN { 
				$$ = new FunctionCallStm($1, reinterpret_cast<std::vector<Expression*>*>($3));
			}
			| IDENTIFIER PLUS PLUS { $$ = new PrePostFixStm($1, true, true); }
			| IDENTIFIER MINUS MINUS { $$ = new PrePostFixStm($1, true, false); }
			| PLUS PLUS IDENTIFIER { $$ = new PrePostFixStm($3, false, true); }
			| MINUS MINUS IDENTIFIER { $$ = new PrePostFixStm($3, false, false); }
			| RETURN expression {
				$$ = new Return(re<Expression>($2));
			}
			;

control_struct:	IF LEFT_PAREN expression RIGHT_PAREN block ELSE block {
						$$ = new If(re<Expression>($3), re<Block>($5), re<Block>($7));
					}
					| IF LEFT_PAREN expression RIGHT_PAREN block {
						$$ = new If(re<Expression>($3), re<Block>($5));
					}
					| FOR LEFT_PAREN statement SEMICOLON expression SEMICOLON statement RIGHT_PAREN block {
						$$ = new ForLoop(re<Statement>($3), re<Expression>($5), re<Statement>($7), re<Block>($9));
					}
					;

expression: INTCONST { $$ = new IntConst($1); }
			| FLOATCONST { $$ = new FloatConst(atof($1), strlen($1) - (strrchr($1, '.') - $1) - 1); }
			| MINUS INTCONST { $$ = new IntConst($2 * -1); }
			| MINUS FLOATCONST { $$ = new FloatConst(atof($2) * -1.0, strlen($2) - (strrchr($2, '.') - $2) - 1); }
			| type_constructable LEFT_PAREN arg_list RIGHT_PAREN {
				$$ = new TypeConstructor(static_cast<Type>($1), reinterpret_cast<std::vector<Expression*>*>($3));
			}
			| IDENTIFIER LEFT_PAREN arg_list RIGHT_PAREN { 
				$$ = new FunctionCall($1, reinterpret_cast<std::vector<Expression*>*>($3));
			}
			| IDENTIFIER { $$ = new Reference($1); }
			/* This allows whitespace, which should be error */
			| expression DOT IDENTIFIER { $$ = new FieldReference(re<Expression>($1), $3); }
			| expression STAR expression { $$ = new MultiplyExpr(re<Expression>($1), re<Expression>($3)); }
			| expression SLASH expression { $$ = new DivideExpr(re<Expression>($1), re<Expression>($3)); }
			| expression PLUS expression { $$ = new PlusExpr(re<Expression>($1), re<Expression>($3)); }
			| expression MINUS expression { $$ = new MinusExpr(re<Expression>($1), re<Expression>($3)); }
			| LEFT_PAREN expression RIGHT_PAREN { $$ = new BracedExpr(re<Expression>($2)); }
			| expression comp_op expression { $$ = new Comparison(static_cast<CompOp>($2), re<Expression>($1), re<Expression>($3)); }
			| MINUS expression { $$ = new UnaryMinus(re<Expression>($2)); }
			;

arg_list:	/* empty */ {
				$$ = new std::vector<Expression*>;
			}
			| arg_list expression {
				$$ = push_node<Expression>($1, $2);
			}
			| arg_list COMMA expression {
				$$ = push_node<Expression>($1, $3);
			}
			;

arg_decl: type IDENTIFIER { $$ = new ArgDecl(itot($1), $2); };

arg_decl_list: /* empty */ {
				$$ = new std::vector<ArgDecl*>;
			}
			| arg_decl_list arg_decl {
				$$ = push_node<ArgDecl>($1, $2);
			}
			| arg_decl_list COMMA arg_decl {
				$$ = push_node<ArgDecl>($1, $3);
			}
			;

type_constructable: VEC2 { $$ = Vec2; }
					| VEC3 { $$ = Vec3; }
					| VEC4 { $$ = Vec4; }
					| MAT2 { $$ = Mat2; }
					| MAT3 { $$ = Mat3; }
					| MAT4 { $$ = Mat4; }
					;

type : VOID { $$ = Void; }
	| INT { $$ = Int; }
	| VEC2 { $$ = Vec2; }
	| VEC3 { $$ = Vec3; }
	| VEC4 { $$ = Vec4; }
	| FLOAT { $$ = Float; }
	| MAT2 { $$ = Mat2; }
	| MAT3 { $$ = Mat3; }
	| MAT4 { $$ = Mat4; }
	;

comp_op : LT	{ $$ = Lt; }
		| GT	{ $$ = Gt; }
		| LTE	{ $$ = Lte; }
		| GTE	{ $$ = Gte; }
		| EQ	{ $$ = Eq; }
		;

assign_op : PLUS EQUAL { $$ = PlusAssign; }
			| MINUS EQUAL { $$ = MinusAssign; }
			| SLASH EQUAL { $$ = DivideAssign; }
			| MODULUS EQUAL { $$ = ModulusAssign; }
			| LEFT_SHIFT EQUAL { $$ = LeftShiftAssign; }
			| RIGHT_SHIFT EQUAL { $$ = RightShiftAssign; }
			| AMPERSAND EQUAL { $$ = BitAndAssign; }
			| CARET EQUAL { $$ = BitOrAssign; }
			| PIPE EQUAL { $$ = BitIncOrAssign; }
%%

void yyerror(const char *s) {
	printf("Parse error on line %d: %s", yylineno, s);
}