enum Type {
    INT, VOID
};
static std::string enumTypeToString(Type item) {
    switch (item) {
    case INT:return "int";
    case VOID:return "void";
    default: return ;
    }
}
class Expression {
};
class Function {
    Type Type;
    std::vector<Expression*>* argExprs;
    std::string identifier;
    Function(Type Type, std::string identifier, std::vector<Expression*>* argExprs) : Type(Type), identifier(identifier), argExprs(argExprs) {}
};
class IdExpr : public Expression {
    std::string identifier;
    IdExpr(std::string identifier) : identifier(identifier) {}
};
class IntExpr : public Expression {
    int intConst;
    IntExpr(int intConst) : intConst(intConst) {}
};
