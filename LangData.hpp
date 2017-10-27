#pragma once
#include "DescrNode.hpp"
#include "DescrVisitor.hpp"
#include <vector>
#include <map>
#include <set>
#include <queue>

namespace LangData {

using std::vector;
using std::string;
using std::map;
using std::set;
using std::queue;

/**
 * Represents a token with key, regex and
 * optionally type
 */
class TokenData {
public:
    TokenType type;
    string key;
    string regex;
    TokenData(TokenType type, string key, string regex)
        : type(type), key(key), regex(regex) {}
};

/**
 * Type of grammar rule part
 */
enum PartType {
    PSTRING,
    PINT,
    PFLOAT,
    PENUM,
    PAST,
    PLIST,
    PTOKEN
};

/**
 * Grammar rule part with derived type information.
 * Tagged with any of PartType enum
 */
class TypedPart {
public:
    PartType type;
    string identifier;
    string alias;
    TypedPart(PartType type, string identifier) : type(type), identifier(identifier) {}
    bool operator==(const TypedPart &other) {
        // Todo, check (ast) expr type
        // or maybe this should be done outside this equality overload
        return (other.type == this->type
            && other.alias.compare(this->alias) == 0);
    }
    bool operator!=(const TypedPart &other) {
        return !(*this == other);
    }
};

// Token part
class TypedPartToken : public TypedPart {
public:
    TypedPartToken(string identifier)
        : TypedPart(PTOKEN, identifier) {}
};
// Prim token part
class TypedPartPrim : public TypedPart {
public:
    TypedPartPrim(PartType type, string identifier)
        : TypedPart(type, identifier) {}
};
// Enum part
class TypedPartEnum : public TypedPart {
public:
    string enumKey;
    TypedPartEnum(string identifier, string enumKey) 
        : TypedPart(PENUM, identifier), enumKey(enumKey) {}
};
// Ast part
class TypedPartAst : public TypedPart {
public:
    string astClass;
    TypedPartAst(string identifier, string astClass) 
        : TypedPart(PAST, identifier), astClass(astClass) {}
};
// List part
class TypedPartList : public TypedPart {
public:
    // Little mismatch to represent all and nested types
    TypedPart *type;
    TypedPartList(string identifier, TypedPart *type) 
        : TypedPart(PLIST, identifier), type(type) {}
};

/**
 * Various actions triggered by rule matches.
 */
enum RuleActionType {
    RAAstConstruction,
    RARef,
    RAEnumValue,
    RAListInit,
    RAListPush
};
class RuleAction {
public:
    RuleActionType type;
    RuleAction(RuleActionType type) : type(type) {}
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
    AstConstructionAction(string astClass) : RuleAction(RAAstConstruction), astClass(astClass)  {}
    AstConstructionAction(string astClass, vector<RuleArg> args)
        : RuleAction(RAAstConstruction), astClass(astClass), args(args) {}
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
};

/**
 * Rule action that sets enum value
 */
class EnumValueAction : public RuleAction {
public:
    string enumMember;
    EnumValueAction(string enumMember) : RuleAction(RAEnumValue), enumMember(enumMember) {}
};

/**
 * Rule action in list types that initialized the list
 */
class ListInitAction : public RuleAction {
public:
    TypedPart *type;
    ListInitAction(TypedPart *type) : RuleAction(RAListInit), type(type) {}
};

/**
 * Rule action in list types that add to list
 */
class ListPushAction : public RuleAction {
public:
    int listNum;
    int elemNum;
    TypedPart *type;
    ListPushAction(int listNum, int elemNum, TypedPart *type)
        : RuleAction(RAListPush), listNum(listNum), elemNum(elemNum), type(type) {}
};

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
    GrammarRule() {}
};

/**
 * Grammar type returning an ast object.
 */
class AstGrammarType {
public:
    string key;
    string astClass;
    vector<GrammarRule*> rules;
    AstGrammarType(string key) : key(key) {}
};

/**
 * Grammar type returning a list
 */
class ListGrammarType {
public:
    string key;
    // Little mismatch to represent all and nested types
    TypedPart *type;
    vector<GrammarRule*> rules;
    ListGrammarType(string key) : key(key) {}
};

/**
 * Grammar type returning enum element
 */
class EnumGrammarType {
public:
    string key;
    string enumKey;
    vector<GrammarRule*> rules;
    EnumGrammarType(string key) : key(key) {}
};

/**
 * Represents a key in an enum in code
 */
class AstEnumMember {
public:
    string name;
    string value;
    AstEnumMember(string name, string value) 
        : name(name), value(value) {}
};

/**
 * Represents an enum in code
 */
class AstEnum {
public:
    string name;
    set<string> members;
    AstEnum(string name) : name(name) {}
};

/**
 * Represents an ast class member in code
 */
class AstClassMember {
public:
    TypedPart *typedPart;
    AstClassMember(TypedPart *typedPart) : typedPart(typedPart) {}
};

/**
 * Represent an ast class constructor in code
 */
class AstClassConstructor {
public:
    vector<string> args;
    AstClassConstructor() {}
};

/**
 * Represents an ast class in code
 */
class AstClass {
public:
    string identifier;
    string extends;
    map<string, AstClassMember*> members;
    vector<AstClassConstructor*> constructors;
    set<string> subClasses;
    AstClass(string identifier) : identifier(identifier) {}
    void ensureMember(TypedPart *typedPart) {
        if (members.count(typedPart->identifier) == 0) {
            members.emplace(typedPart->identifier, new AstClassMember(typedPart));
        } else {
            // Verify type
        }
    }
};

/**
 * Central object for lang data.
 */
class LData {
public:
    map<string, TokenData*> tokenData;
    map<string, AstEnum*> enums;
    set<TokenType> tokenTypes;
    map<string, AstGrammarType*> astGrammarTypes;
    map<string, ListGrammarType*> listGrammarTypes;
    map<string, EnumGrammarType*> enumGrammarTypes;
    map<string, AstClass*> astClasses;
    LData() {}
    AstClass* ensureClass(string className) {
        if (astClasses.count(className) == 0) astClasses.emplace(className, new AstClass(className));
        return astClasses[className];
    }
    AstEnum* ensureEnum(string typeName) {
        if (!enums.count(typeName)) enums.emplace(typeName, new AstEnum(typeName));
        return enums[typeName];
    }
    EnumGrammarType* ensureEnumGrammar(string key) {
        if (!enumGrammarTypes.count(key)) enumGrammarTypes.emplace(key, new EnumGrammarType(key));
        return enumGrammarTypes[key];
    }
    AstGrammarType* ensureAstGrammar(string key) {
        if (!astGrammarTypes.count(key)) astGrammarTypes.emplace(key, new AstGrammarType(key));
        return astGrammarTypes[key];
    }
    ListGrammarType* ensureListGrammar(string key) {
        if (!listGrammarTypes.count(key)) listGrammarTypes.emplace(key, new ListGrammarType(key));
        return listGrammarTypes[key];
    }
    // Ensures sub relationsship, and returns the subclass.
    // Will check for equality and just return base class
    // if equal.
    AstClass* ensureSubRelation(string baseClass, string subClass) {
        AstClass *base = ensureClass(baseClass);
        if (subClass == baseClass) {
            return base;
        } else {
            AstClass *sub = ensureClass(subClass);
            if (sub->extends != "" && sub->extends != baseClass) {
                printf("Todo, handle different base");
                exit(1);
            }
            sub->extends = baseClass;
            base->subClasses.insert(subClass);
            return sub;
        }
    }
    TypedPart* getTypedPart(string identifier) {
        // Check token
        if (tokenData.count(identifier) != 0) {
            TokenData *tokenRef = tokenData[identifier];
            switch (tokenRef->type) {
                case NONE:
                return new TypedPartToken(identifier);
                break;
                case TSTRING:
                return new TypedPartPrim(PSTRING, identifier);
                break;
                case TINT:
                return new TypedPartPrim(PINT, identifier);
                break;
                case TFLOAT:
                return new TypedPartPrim(PFLOAT, identifier);
                break;
            }
        } else if (enumGrammarTypes.count(identifier) != 0) {
            EnumGrammarType *enumData = enumGrammarTypes[identifier];
            return new TypedPartEnum(identifier, enumData->enumKey);
        } else if (astGrammarTypes.count(identifier) != 0) {
            return new TypedPartAst(
                identifier,
                astGrammarTypes[identifier]->astClass
            );
        } else if (listGrammarTypes.count(identifier) != 0) {
            return new TypedPartList(
                identifier,
                listGrammarTypes[identifier]->type
            );
        } else if (tokenData.count(identifier) != 0) {
            return new TypedPartToken(identifier);
        } else {
            return nullptr;
        }
    }
    string keyFromTypeDecl(TypeDecl *typeDecl) {
        return (typeDecl->alias.compare("") != 0) ? typeDecl->alias : typeDecl->identifier;
    }
};

/**
 * Pass to register keys so they are ready when
 * referred to.
 * For example to get a TypedPart, it checks
 * registered tokens, enums, ast and list types.
 * Adds grammar types for ast, list is done in
 * later step.
 * Adds token data, 
 */
class RegisterKeysVisitor : public DescrVisitor {
public:
    LData *langData;
    RegisterKeysVisitor(LData *langData) : langData(langData) {}
    void visitToken(TokenNode *node) {
        langData->tokenData.emplace(node->identifier, new TokenData(
            node->type,
            node->identifier,
            node->regex
        ));
        // Add primitive type to union set
        if (node->type != NONE) {
            langData->tokenTypes.insert(node->type);
        }
    }
    void visitEnum(EnumNode *node) {
        // Register grammartype on the key
        EnumGrammarType *grammar = langData->ensureEnumGrammar(langData->keyFromTypeDecl(node->typeDecl));
        grammar->enumKey = node->typeDecl->identifier;
        DescrVisitor::visitEnum(node);
    }
    // Add enum members as tokens in format
    // KEY "token"
    // Enum keys are atleast tokens.
    // Maybe they could also be other things,
    // but there is also ast types which can
    // take all keys. They return an object
    // by default though. An enum would perhaps
    // sometimes be good.
    void visitEnumDecl(EnumDeclNode *node) {
        langData->tokenData.emplace(node->identifier, new TokenData(
            NONE,
            node->identifier,
            node->regex
        ));
    }
    // Also sets ast base class
    void visitAst(AstNode *node) {
        // Key is given as alias or use class name
        string grammarKey = langData->keyFromTypeDecl(node->typeDecl);
        // Get current grammar
        AstGrammarType *grammar = langData->ensureAstGrammar(grammarKey);
        // Get ast base class from ast declaration
        grammar->astClass = node->typeDecl->identifier;
    }
    void visitList(ListNode *node) {
        // Just register grammar on key,
        // type is set at later stage
        langData->ensureListGrammar(node->identifier);
    }
};

/**
 * Lists are dependent on other keys for their
 * type definition, so here we have the other
 * types and are able to set list keys.
 * Lists of lists todo
 */
class RegisterListKeysVisitor : public DescrVisitor {
public:
    LData *langData;
    queue<ListNode*> unresolved;
    RegisterListKeysVisitor(LData *langData) : langData(langData) {}

    // Runs visitList until there are no more
    // lists in queue, or loop is detected.
    void visitSource(SourceNode *node) {
        DescrVisitor::visitSource(node);
        if (unresolved.size() > 0) {
            size_t prevSize = unresolved.size();
            size_t numTimesEqual = 0;
            while (unresolved.size() > 0) {
                visitList(unresolved.front());
                unresolved.pop();
                if (unresolved.size() == prevSize) {
                    ++numTimesEqual;
                    // Allow equal the same number of times
                    // as there are members in the queue.
                    if (numTimesEqual >= unresolved.size()) {
                        printf("Loop detected in lists\n");
                        while (!unresolved.empty()) {
                            printf("List: %s\n", unresolved.front()->identifier.c_str());
                            unresolved.pop();
                        }
                        exit(1);
                    }
                } else {
                    numTimesEqual = 0;
                }
            }
        }
    }
    void visitList(ListNode *node) {
        ListGrammarType *grammarType = langData->listGrammarTypes[node->identifier];
        TypedPart *typed1 = langData->getTypedPart(node->astKey);
        TypedPart *typed2 = langData->getTypedPart(node->tokenSep);
        // Separator between or at end
        TypedPart *listType;
        TypedPart *sepToken;
        bool sepBetween;
        // Check for a token
        // All tokens should be registered so
        // we should be able to detect a token.
        if (typed1->type == PTOKEN) {
            sepBetween = true;
            sepToken = typed1;
            listType = typed2;
        } else if (typed2->type == PTOKEN) {
            sepBetween = false;
            sepToken = typed2;
            listType = typed1;
        } else {
            printf("List requires a token and a type reference");
            printf("Token before means separator between,\n");
            printf("Token after means separator after each.\n");
            exit(1);
        }
        if (listType == nullptr) {
            // List seems to be needing another list
            // Add to queue that should be
            // passed here again
            unresolved.push(node);
            return;
        }
        grammarType->type = listType;
    }
};
class BuildRulesVisitor : public DescrVisitor {
public:
    LData *langData;
    BuildRulesVisitor(LData *langData) : langData(langData) {}
    void visitEnum(EnumNode *node) {
        // Build rules
        string grammarKey = langData->keyFromTypeDecl(node->typeDecl);
        EnumGrammarType *grammarType = langData->ensureEnumGrammar(grammarKey);
        // Enums are one of several token values
        for (EnumDeclNode *enumDecl : *node->nodes) {
            GrammarRule *rule = new GrammarRule();
            rule->tokenList = vector<string> { enumDecl->identifier };
            rule->action = new EnumValueAction(enumDecl->identifier);
            grammarType->rules.push_back(rule);
        }
    }

    void visitAst(AstNode *node) {
        string grammarKey = langData->keyFromTypeDecl(node->typeDecl);
        // Get current grammar
        AstGrammarType *grammar = langData->ensureAstGrammar(grammarKey);
        // Get ast base class
        string baseAstClass = node->typeDecl->identifier;
        // Go through refs and collect rules
        for (AstDef* astDef : *node->nodes) {
            string defClass = baseAstClass;
            // Defs may have an identifier, this would refer
            // to a subclass of the baseClass or another ast
            // rule. (only next rule is checked, but top level
            // rules should have the type specified)
            // When rules refer to args/parts
            // that is expected in parens.
            bool isKeyRef = false;
            if (astDef->identifier.compare("") != 0) {
                // Check for other ast rule
                if (langData->astGrammarTypes.count(astDef->identifier) != 0) {
                    // Get class name from other type
                    defClass = langData->astGrammarTypes[astDef->identifier]->astClass;
                    isKeyRef = true;
                } else {
                    // Else use identifier as class
                    defClass = astDef->identifier;
                }
            }
            GrammarRule *curRule = new GrammarRule();
            if (isKeyRef) {
                // Rule just need to use referred rule as part
                // which should return ast object
                curRule->tokenList.push_back(astDef->identifier);
                curRule->action = new RefAction(1, new TypedPartAst(
                    astDef->identifier,
                    defClass
                ));
            } else {
                // Collect rule args
                vector<RuleArg> ruleArgs;
                int num = 0;
                vector<string> tokenList;
                for (AstPart *part : *astDef->nodes) {
                    ++num;
                    TypedPart *typedPart = langData->getTypedPart(part->identifier);
                    // Just setting alias here
                    // Used as key to ast member and constructor args
                    typedPart->alias = (part->alias != "") ? part->alias : part->identifier;
                    if (typedPart == nullptr) {
                        printf("Key not found: %s\n", part->identifier.c_str());
                        exit(1);
                    }
                    tokenList.push_back(typedPart->identifier);
                    if (typedPart->type == PTOKEN || typedPart == nullptr) {
                        // Skip const literal tokens
                        continue;
                    }
                    // Add rule arg
                    ruleArgs.push_back(RuleArg(num, typedPart));
                }
                // Add rule to grammar type
                curRule->tokenList = tokenList;
                printf("Adding constr action: %s\n", defClass.c_str());
                curRule->action = new AstConstructionAction(
                    defClass,
                    ruleArgs
                );
            }
            grammar->rules.push_back(curRule);
        }
    }
    void visitList(ListNode *node) {
        ListGrammarType *grammar = langData->ensureListGrammar(node->identifier);
        TypedPart *typed1 = langData->getTypedPart(node->astKey);
        TypedPart *typed2 = langData->getTypedPart(node->tokenSep);
        if (typed1 == nullptr || typed2 == nullptr) {
            printf("List key not found");
            exit(1);
        }
        // Separator between or at end
        TypedPart *listType;
        TypedPart *sepToken;
        bool sepBetween;
        if (typed1->type == PTOKEN) {
            sepBetween = true;
            sepToken = typed1;
            listType = typed2;
        } else {
            sepBetween = false;
            sepToken = typed2;
            listType = typed1;
        }
        grammar->type = listType;
        // Init list
        GrammarRule *initRule = new GrammarRule();
        initRule->action = new ListInitAction(listType);
        grammar->rules.push_back(initRule);
        if (sepBetween) {
            GrammarRule *firstPart = new GrammarRule();
            firstPart->tokenList = vector<string>{
                node->identifier,
                listType->identifier
            };
            firstPart->action = new ListPushAction(1, 2, listType);
            grammar->rules.push_back(firstPart);
            GrammarRule *sepPart = new GrammarRule();
            sepPart->tokenList = vector<string>{
                node->identifier,
                sepToken->identifier,
                listType->identifier
            };
            sepPart->action = new ListPushAction(1, 3, listType);
            grammar->rules.push_back(sepPart);
        } else {
            GrammarRule *sepPart = new GrammarRule();
            sepPart->tokenList = vector<string>{
                node->identifier,
                listType->identifier,
                sepToken->identifier
            };
            sepPart->action = new ListPushAction(1, 2, listType);
            grammar->rules.push_back(sepPart);
        }
    }
};

class BuildAstVisitor : public DescrVisitor {
public:
    LData *langData;
    BuildAstVisitor(LData *langData) : langData(langData) {}
    void visitEnum(EnumNode *node) {
        AstEnum* astEnum = langData->ensureEnum(node->typeDecl->identifier);
        for (EnumDeclNode *enumDecl : *node->nodes) {
            astEnum->members.insert(enumDecl->identifier);
        }
    }

    void visitAst(AstNode *node) {
        string grammarKey = langData->keyFromTypeDecl(node->typeDecl);
        // Get current grammar
        AstGrammarType *grammar = langData->ensureAstGrammar(grammarKey);
        // Get ast base class
        string baseAstName = node->typeDecl->identifier;
        langData->ensureClass(baseAstName);
        // Go through rules and ensure ast classes
        // has needed members and constructors
        for (GrammarRule *rule : grammar->rules) {
            // If rule is a ref, register referenced
            // ast class as subclass.
            // Members and constructors are set up when
            // visiting referenced ast node.
            if (rule->action->type == RARef) {
                RefAction *refAction = static_cast<RefAction*>(rule->action);
                if (refAction->ref->type != PAST) {
                    printf("Can only handle ref to ast");
                    exit(1);
                }
                // Ensure subclass relationship
                TypedPartAst *refType = static_cast<TypedPartAst*>(refAction->ref);
                langData->ensureSubRelation(baseAstName, refType->astClass);
                continue;
            } else if (rule->action->type != RAAstConstruction) {
                printf("Can only handle ref and ast construction actions");
                exit(1);
            }
            AstConstructionAction *action = static_cast<AstConstructionAction*>(rule->action);
            AstClass *ruleClass = langData->ensureSubRelation(baseAstName, action->astClass);
            // Ensure class has members for all args
            for (RuleArg ruleArg : action->args) {
                TypedPart *typedPart = ruleArg.typedPart;
                if (ruleClass->members.count(typedPart->alias) != 0) {
                    // This equality test will check identifier, alias and type
                    // It should probably test type more thoroughly, and
                    // rather check resolved key todo
                    if (*ruleClass->members[typedPart->alias]->typedPart != *typedPart) {
                        // Member has different type
                        printf("Member has different type");
                        exit(1);
                    }
                } else {
                    ruleClass->members.emplace(
                        typedPart->alias,
                        new AstClassMember(typedPart)
                    );
                }
            }
            // Ensure ast class constructor
            bool constructorFound = false;
            for (AstClassConstructor *constr : ruleClass->constructors) {
                if (constr->args.size() == action->args.size()) {
                    // Do simple equality check.
                    // Concievably we could try to reorder, but
                    // that could also lead to more volatility
                    // in available constructors
                    // Perhaps when the ast interface is defined,
                    // reorder could be tried.
                    bool isEqual = true;
                    for (size_t i = 0; i < constr->args.size(); ++i) {
                        // Assume correspondance with member field types
                        if (constr->args[i] != action->args[i].typedPart->alias) {
                            isEqual = false;
                            break;
                        }
                    }
                    if (isEqual) {
                        constructorFound = true;
                        break;
                    }
                }
            }
            if (!constructorFound) {
                // Create constructor based on ruleArgs
                AstClassConstructor *constr = new AstClassConstructor();
                for (RuleArg ruleArg : action->args) {
                    constr->args.push_back(ruleArg.typedPart->alias);
                }
                ruleClass->constructors.push_back(constr);
            }
        }
    }
};

}