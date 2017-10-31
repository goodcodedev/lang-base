#include "RuleAction.hpp"

namespace LangBase {

void RuleAction::generateGrammar(string *str, LData *langData) {
    *str += "$$ = ";
    generateGrammarVal(str, langData);
    *str += ";";
}

void AstConstructionAction::generateGrammarVal(string *str, LData *langData) {
    *str += "new " + astClass + "(";
    bool isFirst = true;
    for (RuleArg &arg : args) {
        if (!isFirst) *str += ", ";
        arg.typedPart->generateGrammarVal(str, arg.num, langData);
        isFirst = false;
    }
    *str += ")";
}

void RefAction::generateGrammarVal(string *str, LData *langData) {
    *str += "$" + std::to_string(num);
}

void EnumValueAction::generateGrammarVal(string *str, LData *langData) {
    *str += enumMember;
}

void ListInitAction::generateGrammarVal(string *str, LData *langData) {
    *str += "new std::vector<";
    type->generateGrammarType(str, langData);
    *str += ">";
}

void ListPushAction::generateGrammar(string *str, LData *langData) {
    // Reinterpret as list type to "vec" variable
    *str += "std::vector<";
    type->generateGrammarType(str, langData);
    *str += ">* vec = reinterpret_cast<std::vector<";
    type->generateGrammarType(str, langData);
    *str += ">*>($" + std::to_string(listNum) + ");";
    // Push back element
    *str += "vec->push_back(";
    type->generateGrammarVal(str, elemNum, langData);
    *str += ");$$ = ";
    generateGrammarVal(str, langData);
    *str += ";";
}
void ListPushAction::generateGrammarVal(string *str, LData *langData) {
    *str += "vec";
}

void StartAction::generateGrammar(string *str, LData *langData) {
    *str += "result = ";
    generateGrammarVal(str, langData);
    *str += ";$$ = result;";
}
void StartAction::generateGrammarVal(string *str, LData *langData) {
    startPart->generateGrammarVal(str, 1, langData);
}
}