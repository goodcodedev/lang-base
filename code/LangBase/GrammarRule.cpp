#include "GrammarRule.hpp"

namespace LangBase {

void GrammarRule::generateGrammar(string *str, LData *langData) {
    for (string token : tokenList) {
        TypedPart *typed = langData->getTypedPart(token);
        if (typed->identifier == "WS") {
            // Ignore WS token used to
            // signal whitespace is needed
            // in source
            continue;
        }
        *str += " " + typed->getGrammarToken();
    }
    *str += " { ";
    action->generateGrammar(str, langData);
    *str += " }";
}

void GrammarType::generateGrammar(string *str, LData *langData) {
    *str += key + ":";
    bool isFirst = true;
    for (GrammarRule *rule : rules) {
        if (!isFirst) *str += "\n    |";
        rule->generateGrammar(str, langData);
        isFirst = false;
    }
    *str += "\n    ;\n";
}
}