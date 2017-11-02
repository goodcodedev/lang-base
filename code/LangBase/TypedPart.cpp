#include <stdlib.h>
#include "TypedPart.hpp"
#include "LangData.hpp"
#include "Process/ToSourceGenVisitor.hpp"

namespace LangBase {


void TypedPartToken::generateGrammarVal(string *str, int num, LData *langData) {
    // Its not normal to pass a token,
    // but it could be useful to pass a constant
    // when matching something for example.
    // It is passed as the string
    // that was set up with the token key
    if (langData->tokenData.count(identifier) == 0) {
        printf("Token not found");
        exit(1);
    }
    *str += "\"" + langData->tokenData[identifier]->regex + "\"";
}
void TypedPartToken::generateGrammarType(string *str, LData *langData) {
    *str += "std::string";
}
void TypedPartToken::addToVisitor(ToSourceCase *visitor) {
    string regex = visitor->langData->tokenData[identifier]->regex;
    // Currently handles simple regexes
    // If the regex result is variable, it needs to
    // be captured and put into ast class
    string cleaned;
    // Token can be the WS token containing a space
    // and meant to signal whitespace is needed
    // in source code.
    for (size_t i = 0; i < regex.size(); ++i) {
        if (regex[i] != '\\') {
            cleaned += regex[i];
        }
    }
    visitor->code += "str += \"" + cleaned + "\";\n";
}

void TypedPartPrim::generateGrammarVal(string *str, int num, LData *langData) {
    *str += "$" + std::to_string(num);
}
void TypedPartPrim::generateGrammarType(string *str, LData *langData) {
    switch (type) {
        case PSTRING:
        *str += "std::string";
        break;
        case PINT:
        *str += "int";
        break;
        case PFLOAT:
        *str += "double";
        break;
        default:
        *str += "UNRECOGNIZED PRIM TYPE";
    }
}
void TypedPartPrim::addToVisitor(ToSourceCase *visitor) {
    switch (type) {
        case PSTRING:
        visitor->code += "    str += node->" + getMemberKey() + ";\n";
        break;
        case PINT:
        visitor->code += "    str += std::to_string(node->" + getMemberKey() + ");\n";
        break;
        case PFLOAT:
        visitor->code += "    str += std::to_string(node->" + getMemberKey() + ");\n";
        break;
        default: {
            printf("Unrecognized prim type in addToVisitor\n");
            exit(1);
        }
    }
}

void TypedPartEnum::generateGrammarVal(string *str, int num, LData *langData) {
    *str += "static_cast<" + enumKey + ">($" + std::to_string(num) + ")";
}
void TypedPartEnum::generateGrammarType(string *str, LData *langData) {
    *str += enumKey;
}
void TypedPartEnum::addToVisitor(ToSourceCase *visitor) {
    visitor->code += "str += enum" + identifier + "ToString(node->" + getMemberKey() + ");\n";
}

void TypedPartAst::generateGrammarVal(string *str, int num, LData *langData) {
    *str += "reinterpret_cast<" + astClass + "*>($" + std::to_string(num) + ")";
}
void TypedPartAst::generateGrammarType(string *str, LData *langData) {
    *str += astClass + "*";
}
void TypedPartAst::addToVisitor(ToSourceCase *visitor) {
    if (visitor->isClassKey) {
        visitor->code += "    visit" + astClass + "(node->" + getMemberKey() + ");\n";
    } else {
        visitor->code += "    astKey_" + visitor->grammarKey + "(node->" + getMemberKey() + ");\n";
    }
}

void TypedPartList::generateGrammarVal(string *str, int num, LData *langData) {
    *str += "reinterpret_cast<";
    generateGrammarType(str, langData);
    *str += ">($" + std::to_string(num) + ")";
}
void TypedPartList::generateGrammarType(string *str, LData *langData) {
    *str += "std::vector<";
    type->generateGrammarType(str, langData);
    *str += ">*";
}
void TypedPartList::addToVisitor(ToSourceCase *visitor) {
    visitor->code += "    listKey_" + visitor->grammarKey + "(node->" + getMemberKey() + ");\n";
    return;
    if (type->type == PAST) {
        TypedPartAst *astType = static_cast<TypedPartAst*>(type);
        visitor->code += "for (";
        type->generateGrammarType(&visitor->code, visitor->langData);
        string memberKey = getMemberKey();
        visitor->code += " child : *node->" + memberKey + ") {\n";
        if (sepBetween) {
            visitor->code += "if (child != node->" + memberKey + "->front()) {\n";
            sep->addToVisitor(visitor);
            visitor->code += "}\n";
        }
        visitor->code += "visit" + astType->astClass + "(child);\n";
        if (!sepBetween) {
            sep->addToVisitor(visitor);
        }
        visitor->code += "}\n";
    } else {
        printf("Currently only ast supported in lists\n");
        exit(1);
    }
}
}