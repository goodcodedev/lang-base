#pragma once
#include <string>
#include <vector>
#include "RuleAction.hpp"
#include "TypedPart.hpp"

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

/**
 * Describing ast rule with available metadata
 * Possibly there could be better datatypes
 * for ruledefs. Starting with this
 */
class AstRuleDef {
public:
    vector<string> tokenList;
    vector<TypedPart*> typedPartList;
    // Base class or class specified on the rule
    string astClass;
    string baseClass;
    TypedPartAst *refType;
    string serialized;
};

class EnumRuleDef {
public:
    vector<string> tokenList;
    string serialized;
    string enumType;
};

/**
 * Describing a listrule
 * Slightly different from grammar rules,
 * as they may have extra rule to facilitate
 * creating and adding to a vector.
 * More similar to/next step after ast ListDef.
 * Maybe ListDef could be used, but
 * most datastructures here are processed
 * from ast, so making another to
 * follow in that track.
 * When shorthand form is used,
 * there are no list rules.
 * Consider making dedicated data
 * structures for shorthand.
 */
class ListRuleDef {
public:
    // Optional separator after item
    TypedPart *sepAfter;
    // Optional separator between items
    TypedPart *sepBetween;
    // Token list with separator
    vector<string> tokenList;
    vector<TypedPart*> typedPartList;
    // Local class for this rule,
    // or base class of the type
    string serialized;
    // When ast rule contained
    AstRuleDef *astRule;
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
    vector<AstRuleDef*> ruleDefs;
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
    // List of rules. Empty on shorthand form
    vector<ListRuleDef*> ruleDefs;
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