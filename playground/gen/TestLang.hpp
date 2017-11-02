#pragma once
#include <string>
#include <vector>
enum NodeType {
    AssignNode, ControlStructNode, ExpressionNode, FunctionNode, IdExprNode, IfNode, IntExprNode, StatementNode
};
enum Type {
    INT, VOID
};
static std::string enumTypeToString(Type item) {
    switch (item) {
    case INT:return "int";
    case VOID:return "void";
    default: return "";
    }
}
class AstNode {
public:
    NodeType nodeType;
    AstNode(NodeType nodeType) : nodeType(nodeType) {}
    virtual ~AstNode() {}
};
class Assign;
class ControlStruct;
class Expression;
class Function;
class IdExpr;
class If;
class IntExpr;
class Statement;
class Statement : public AstNode {
public:
    Statement(NodeType nodeType) : AstNode(nodeType) {}
};
class Assign : public Statement {
public:
    Expression* expr;
    std::string identifier;
    Assign(std::string identifier, Expression* expr) : Statement(AssignNode), expr(expr), identifier(identifier) {}
};
class ControlStruct : public Statement {
public:
    ControlStruct(NodeType nodeType) : Statement(nodeType) {}
};
class Expression : public AstNode {
public:
    Expression(NodeType nodeType) : AstNode(nodeType) {}
};
class Function : public AstNode {
public:
    std::vector<Expression*>* argExprs;
    std::string identifier;
    std::vector<Statement*>* statements;
    Type type;
    Function(Type type, std::string identifier, std::vector<Expression*>* argExprs, std::vector<Statement*>* statements) : AstNode(FunctionNode), argExprs(argExprs), identifier(identifier), statements(statements), type(type) {}
};
class IdExpr : public Expression {
public:
    std::string identifier;
    IdExpr(std::string identifier) : Expression(IdExprNode), identifier(identifier) {}
};
class If : public ControlStruct {
public:
    Expression* expr;
    If(Expression* expr) : ControlStruct(IfNode), expr(expr) {}
};
class IntExpr : public Expression {
public:
    int intConst;
    IntExpr(int intConst) : Expression(IntExprNode), intConst(intConst) {}
};
extern FILE *yyin;
extern int yyparse();
extern Function* result;
class Loader {
public:
static Function* parseFile(std::string fileName) {
   FILE *sourceFile;
   #ifdef _WIN32
   fopen_s(&sourceFile, fileName.c_str(), "r");
   #else
   sourceFile = fopen(fileName.c_str(), "r");
   #endif
   if (!sourceFile) {
       printf("Can't open file %s", fileName.c_str());
       exit(1);
   }
   yyin = sourceFile;
   do {
       yyparse();
   } while (!feof(yyin));
   return result;
}
};
