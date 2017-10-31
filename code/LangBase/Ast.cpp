#include "Ast.hpp"
#include <stdlib.h>

namespace LangBase {
// Code generation
void AstEnum::generateDefinition(string *str, LData *langData) {
    *str += "enum " + name + " {\n    ";
    bool isFirst = true;
    for (string member : members) {
        if (!isFirst) *str += ", ";
        *str += member;
        isFirst = false;
    }
    *str += "\n};\n";
}
void AstEnum::generateToStringMethod(string *str, LData *langData) {
    *str += "static std::string enum" + name + "ToString(" + name + " item) {\n";
    *str += "    switch (item) {\n";
    for (string member : members) {
        *str += "    case " + member + ":";
        *str += "return \"" + values[member] + "\";\n";
    }
    *str += "    default: return \"\";\n";
    *str += "    }\n}\n";
}
void AstClassMember::generateMember(string *str, LData *langData, AstClass *astClass) {
    *str += "    ";
    typedPart->generateGrammarType(str, langData);
    *str += " " + typedPart->getMemberKey() + ";\n";
}
void AstClassConstructor::generateConstructor(string *str, LData *langData, AstClass *astClass) {
    *str += "    ";
    *str += astClass->identifier + "(";
    size_t numArgs = args.size();
    for (size_t i = 0; i < numArgs; ++i) {
        if (astClass->members.count(args[i]) == 0) {
            printf("Member not found");
            exit(1);
        }
        AstClassMember *member = astClass->members[args[i]];
        member->typedPart->generateGrammarType(str, langData);
        *str += " " + args[i];
        if (i + 1 < numArgs) {
            *str += ", ";
        }
    }
    *str += ")";
    // Parent constructor, initialization list
    *str += " : ";
    // Pass node type enum to parent class
    string classEnum = astClass->identifier + "Node";
    if (astClass->extends != "") {
        *str += astClass->extends + "(" + classEnum + ")";
    } else {
        *str += "AstNode(" + classEnum + ")";
    }
    // Initialize in same order as members
    vector<int> order;
    for (auto const &pair : astClass->members) {
        // Check for arg, and add to order vector
        // if found
        for (size_t i = 0; i < numArgs; ++i) {
            if (args[i] == pair.first) {
                order.push_back(i);
                break;
            }
        }
    }
    if (order.size() != numArgs) {
        printf("Didn't find members for all args\n");
        exit(1);
    }
    for (size_t i = 0; i < numArgs; ++i) {
        *str += ", ";
        string arg = args[order[i]];
        *str += arg + "(" + arg + ")";
    }
    *str += " {}\n";
}
void AstClass::generateHeader(string *str, LData *langData) {
    *str += "class " + identifier;
    if (extends != "") *str += " : public " + extends;
    else *str += " : public AstNode";
    *str += " {\npublic:\n";
    for (auto const &member : members) {
        member.second->generateMember(str, langData, this);
    }
    for (AstClassConstructor *constr : constructors) {
        constr->generateConstructor(str, langData, this);
    }
    if (subClasses.size() > 0) {
        // Generate constructor with node type
        *str += "    " + identifier + "(NodeType nodeType) : ";
        if (extends != "") {
            *str += extends + "(nodeType)";
        } else {
            *str += "AstNode(nodeType)";
        }
        *str += " {}\n";
    }
    *str += "};\n";
}
void AstClass::generateDefinition(string *str, LData *langData) {

}
}