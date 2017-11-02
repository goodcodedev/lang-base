#include "TestLangVisitor.hpp"
#include <string>
class TestLangToSource : public TestLangVisitor {
public:
std::string str;
void visitControlStruct(ControlStruct *node) {
    visitExpression(node->expr);

}
void visitFunction(Function *node) {
    str += enumTypeToString(node->type);
str += " ";
str += node->identifier;
str += "(";
visitListKey_Function(node->argExprs);
str += ")";
str += "{";
visitListKey_Function(node->statements);
str += "}";

}
void visitIntExpr(IntExpr *node) {
    str += std::to_string(node->intConst);

}
void visitAstKey_Expression(Expression *node) {
switch (node->serialized) {
    case : {    visitAstKey_expr(node->intExpr);

    }
    break;
    case : {    str += node->identifier;

    }
    break;
}
switch (node->serialized) {
    case : {    visitAstKey_expr(node->intExpr);

    }
    break;
    case : {    str += node->identifier;

    }
    break;
}
}
void visitListKey_argExprs(argExprs *node) {
    
}
void visitListKey_statements(statements *node) {
switch (node->serialized) {
    case : {    str += node->identifier;
str += "=";
visitAstKey_statements(node->expr);

    }
    break;
    case : {    visitAstKey_statements(node->controlStruct);

    }
    break;
}
switch (node->serialized) {
    case : {    str += node->identifier;
str += "=";
visitAstKey_statements(node->expr);

    }
    break;
    case : {    visitAstKey_statements(node->controlStruct);

    }
    break;
}
}
};
