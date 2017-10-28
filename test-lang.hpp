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
Function* parseFile(std::string fileName) {
   FILE *sourceFile;
   errno = 0;
   string fullFile = std::string(PROJECT_ROOT) + fileName;
   #ifdef _WIN32
   fopen_s(&sourceFile, fullFile.c_str(), "r");
   #else
   sourceFile = fopen(fullFile.c_str(), "r");
   #endif
   if (!sourceFile) {
       printf("Can't open file %d", errno);
       exit(1);
   }
   yyin = sourceFile;
   do {
       yyparse();
   } while (!feof(yyin));
   return result;
}
