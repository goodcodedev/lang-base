#pragma once
#include <string>
#include <vector>
#include "RuleAction.hpp"
#include "TypedPart.hpp"
#include "LangData.hpp"

namespace LangBase {
    
using std::string;
using std::vector;

class RuleAction;
class LData;

/**
 * One grammar rule (possibly out of several alternatives) 
 * belonging to a grammar type.
 * The rule consists of a list of parts (referencing identifiers),
 * and an action to run on rule match.
 */
class GrammarRule {
public:
    vector<string> tokenList;
    RuleAction *action;
    string serialized;
    GrammarRule() {}
    void generateGrammar(string *str, LData *langData);
};

class GrammarType {
public:
    string key;
    vector<GrammarRule*> rules;
    GrammarType(string key) : key(key) {}
    void generateGrammar(string *str, LData *langData);
};

/**
 * Grammar type returning an ast object.
 */
class AstGrammarType : public GrammarType {
public:
    string astClass;
    AstGrammarType(string key) : GrammarType(key) {}
};

/**
 * Grammar type returning a list
 */
class ListGrammarType : public GrammarType {
public:
    // Little mismatch to represent all and nested types
    TypedPart *type;
    TypedPart *sep;
    bool sepBetween;
    ListGrammarType(string key) : GrammarType(key) {}
};

/**
 * Grammar type returning enum element
 */
class EnumGrammarType : public GrammarType {
public:
    string enumKey;
    EnumGrammarType(string key) : GrammarType(key) {}
};
}