#pragma once
#include <string>
#include <vector>
#include "TypedPart.hpp"

namespace LangBase {

using std::string;
using std::vector;

class LData;

/**
 * Various actions triggered by rule matches.
 */
enum RuleActionType {
    RAAstConstruction,
    RARef,
    RAEnumValue,
    RAListInit,
    RAListPush,
    RAStart
};
class RuleAction {
public:
    RuleActionType type;
    RuleAction(RuleActionType type) : type(type) {}
    virtual void generateGrammar(string *str, LData *langData);
    virtual void generateGrammarVal(string *str, LData *langData) = 0;
};

/**
 * Argument used by rule actions which
 * reference parts by num
 */
class RuleArg {
public:
    int num;
    TypedPart *typedPart;
    RuleArg(int num, TypedPart *typedPart) : num(num), typedPart(typedPart) {}
};

/**
 * Rule action that creates ast object
 */
class AstConstructionAction : public RuleAction {
public:
    string astClass;
    vector<RuleArg> args;
    string serialized;
    TypedPart *typed;
    AstConstructionAction(string astClass) : RuleAction(RAAstConstruction), astClass(astClass)  {}
    AstConstructionAction(string astClass, vector<RuleArg> args)
        : RuleAction(RAAstConstruction), astClass(astClass), args(args) {}
    AstConstructionAction(string astClass, vector<RuleArg> args, string serialized)
        : RuleAction(RAAstConstruction), astClass(astClass), args(args), serialized(serialized) {}
    void generateGrammarVal(string *str, LData *langData);
};

/**
 * Reference to part, passing it as current element
 * This can be useful in list of rules.
 */
class RefAction : public RuleAction {
public:
    int num;
    TypedPart *ref;
    RefAction(int num, TypedPart *ref) : RuleAction(RARef), num(num), ref(ref) {}
    void generateGrammarVal(string *str, LData *langData);
};

/**
 * Rule action that sets enum value
 */
class EnumValueAction : public RuleAction {
public:
    string enumMember;
    EnumValueAction(string enumMember) : RuleAction(RAEnumValue), enumMember(enumMember) {}
    void generateGrammarVal(string *str, LData *langData);
};

/**
 * Rule action in list types that initialized the list
 */
class ListInitAction : public RuleAction {
public:
    TypedPart *type;
    ListInitAction(TypedPart *type) : RuleAction(RAListInit), type(type) {}
    void generateGrammarVal(string *str, LData *langData);
};

/**
 * Rule action in list types that add to list
 */
class ListPushAction : public RuleAction {
public:
    int listNum;
    int elemNum;
    RuleAction *innerAction;
    TypedPart *type;
    TypedPart *listType;
    ListPushAction(int listNum, int elemNum, TypedPart *type, TypedPart *listType)
        : RuleAction(RAListPush), listNum(listNum), elemNum(elemNum), type(type), listType(listType) {}
    ListPushAction(int listNum, RuleAction *innerAction, TypedPart *type, TypedPart *listType)
        : RuleAction(RAListPush), listNum(listNum), innerAction(innerAction), type(type), listType(listType) {}
    void generateGrammar(string *str, LData *langData);
    void generateGrammarVal(string *str, LData *langData);
};

class StartAction : public RuleAction {
public:
    TypedPart *startPart;
    StartAction(TypedPart *startPart) : RuleAction(RAStart), startPart(startPart) {}
    void generateGrammar(string *str, LData *langData);
    void generateGrammarVal(string *str, LData *langData);
};
}