#include "TestLangVisitor.hpp"
#include <string>
class TestLangToSource : public TestLangVisitor {
public:
    std::string str;
    void astKey_ControlStruct(ControlStruct *node);
    void astKey_Function(Function *node);
    void astKey_IntExpr(IntExpr *node);
    void astKey_expr(Expression *node);
    void listKey_argExprs(std::vector<Expression*> *list);
    void listKey_statements(std::vector<Statement*> *list);
    void visitAssign(Assign *node);
    void visitFunction(Function *node);
    void visitIdExpr(IdExpr *node);
    void visitIf(If *node);
    void visitIntExpr(IntExpr *node);
};

void TestLangToSource::astKey_ControlStruct(ControlStruct *node) {
    switch (node->nodeType) {
        case IfNode: visitIf(static_cast<If*>(node));break;
    }
}
void TestLangToSource::astKey_Function(Function *node) {
    switch (node->nodeType) {
        case FunctionNode: visitFunction(static_cast<Function*>(node));break;
    }
}
void TestLangToSource::astKey_IntExpr(IntExpr *node) {
    switch (node->nodeType) {
        case IntExprNode: visitIntExpr(static_cast<IntExpr*>(node));break;
    }
}
void TestLangToSource::astKey_expr(Expression *node) {
    switch (node->nodeType) {
        case IntExprNode:
        astKey_IntExpr(static_cast<IntExpr*>(node));break;
        case IdExprNode: visitIdExpr(static_cast<IdExpr*>(node));break;
    }
}
void TestLangToSource::listKey_argExprs(std::vector<Expression*> *nodes) {
    for (Expression* node : *nodes) {
        if (node != nodes->front()) {
            str += ",";
        }
        switch (node->nodeType) {
            case IdExprNode:
            case IntExprNode:
            {
                astKey_expr(static_cast<Expression*>(node));
                break;
            }
        }
    }
}
void TestLangToSource::listKey_statements(std::vector<Statement*> *nodes) {
    for (Statement* node : *nodes) {
        switch (node->nodeType) {
            case AssignNode: {
                visitAssign(static_cast<Assign*>(node));
                str += ";";
                break;
            }
            case IfNode:
            {
                astKey_ControlStruct(static_cast<ControlStruct*>(node));
                break;
            }
        }
    }
}
void TestLangToSource::visitAssign(Assign *node) {
    str += node->identifier;
    str += "=";
    astKey_expr(node->expr);

}
void TestLangToSource::visitFunction(Function *node) {
    str += enumTypeToString(node->type);
    str += " ";
    str += node->identifier;
    str += "(";
    listKey_argExprs(node->argExprs);
    str += ")";
    str += "{";
    listKey_statements(node->statements);
    str += "}";

}
void TestLangToSource::visitIdExpr(IdExpr *node) {
    str += node->identifier;

}
void TestLangToSource::visitIf(If *node) {
    visitExpression(node->expr);

}
void TestLangToSource::visitIntExpr(IntExpr *node) {
    str += std::to_string(node->intConst);

}
