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
    PartTypeBase(ParType type, string identifier) : type(type), identifier(identifier) {}
    bool operator==(const TypedPart &other) {
        return (other->type == this->type && other->identifier.compare(this->identifier) == 0);
    }
    bool operator!=(const TypedPart &other) {
        return !(*this == other);
    }
};

// Token part
class TypedPartToken {
public:
    TypedPartToken(string identifier)
        : TypedPart(PTOKEN, identifier) {}
};
// Prim token part
class TypedPartPrim {
public:
    TypedPartPrim(PartType type, string identifier)
        : TypedPart(type, identifier) {}
};
// Enum part
class TypedPartEnum {
public:
    string enumKey;
    TypedPartEnum(string identifier, string enumKey) 
        : TypedPart(PENUM, identifier), enumKey(enumKey) {}
};
// Ast part
class TypedPartAst {
public:
    string astClass;
    TypedPartAst(string identifier, string astClass) 
        : TypedPart(PAST, identifier), astClass(astClass) {}
};
// List part
class TypedPartList {
public:
    // Little mismatch to represent all and nested types
    TypedPart *type;
    TypedPartList(string identifier, TypedPart *type) 
        : TypedPart(PLIST, identifier), type(type) {}
};

/**
 * Various actions triggered by rule matches.
 */
class RuleAction {};

/**
 * Argument used by rule actions which
 * reference parts by num
 */
class RuleArg {
public:
    int num;
    TypedPart *typedPart;
    RuleArg(int num, TypedPart *typePart) : num(num), typedPart(typedPart) {}
};

/**
 * Rule action that creates ast object
 */
class AstConstructionAction : public RuleAction {
public:
    string astClass;
    vector<RuleArg> args;
    AstConstructionAction(string astClass) : astClass(astClass) {}
};

/**
 * Rule action that sets enum value
 */
class EnumValueAction : public RuleAction {
public:
    string enumMember;
    EnumValueAction(string enumMember) : enumMember(enumMember) {}
};

/**
 * Rule action in list types that initialized the list
 */
class ListInitAction : public RuleAction {
public:
    TypedPart *type;
    ListInitAction(TypedPart *type) : type(type) {}
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
        : listNum(listNum), elemNum(elemNum), type(type) {}
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
    string string;
    AstEnumMember(string name, string string) 
        : name(name), string(string) {}
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
    AstClass() {}
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
class LangData {
public:
    map<string, TokenData*> tokenData;
    map<string, AstEnum*> enums;
    set<TokenType> tokenTypes;
    map<string, AstGrammarType*> astGrammarTypes;
    map<string, ListGrammarType*> listGrammarTypes;
    map<string, EnumGrammarType*> enumGrammarTypes;
    map<string, AstClass*> astClasses;
    LangData() {}
    AstClass* ensureClass(string className) {
        if (astClasses.count(className) == 0) astClasses.emplace(className, new AstClass());
        return astClasses[className];
    }
    AstEnum* ensureEnum(string typeName) {
        if (!enums.count(typeName)) enums.emplace(typeName, new AstEnum(typeName));
        return enums[typeName];
    }
    EnumGrammarType* ensureEnumGrammar(string key) {
        if (!enumGrammarTypes.count(key)) enumGrammarTypes.emplace(key, new AstGrammarType(key));
        return enumGrammarTypes[key];
    }
    AstGrammarType* ensureAstGrammar(string key) {
        if (!astGrammarTypes.count(key)) enums.emplace(key, new AstGrammarType(key));
        return astGrammarTypes[key];
    }
    ListGrammarType* ensureListGrammar(string key) {
        if (!listGrammarTypes.count(key)) listGrammarTypes.emplace(key, new ListGrammarType(key));
        return listGrammarTypes[key];
    }
    TypedPart* getTypedPart(AstPart *astPart) {
        // Check token
        if (tokenData.count(part->identifier) != 0) {
            TokenData *tokenRef = tokenData[part->identifier];
            switch (tokenRef->type) {
                case NONE:
                return new TypedPartToken(part->identifier);
                break;
                case TSTRING:
                return new TypedPartPrim(PSTRING, part->identifier);
                break;
                case TINT:
                return new TypedPartPrim(PINT, part->identifier);
                break;
                case TFLOAT:
                return new TypedPartPrim(PFLOAT, part->identifier);
                break;
            }
        } else if (enums.count(part->identifier) != 0) {
            AstEnum *enumData = enums[part->identifier];
            return new TypedPartEnum(part->identifier, enumData->name);
        } else if (astGrammarTypes.count(part->identifier) != 0) {
            return new TypedPartAst(
                part->identifier,
                astGrammarTypes[part->identifier]->astClass
            )
        } else if (listGrammarTypes.count(part->identifier) != 0) {
            return new TypedPartList(
                part->identifier,
                listGrammarTypes[part->identifier]->type
            )
        } else if (tokenData.count(part->identifier) != 0) {
            return new TypedPartToken(part->identifier);
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
    LangData *langData;
    RegisterKeysVisitor(LangData *langData) : langData(langData) {}
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
        langData->ensureEnumGrammar(langData->keyFromTypeDecl(node->typeDecl));
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
        langData->tokenData->emplace(node->identifier, new TokenData(
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
        ListGrammarType *grammarType =  langData->ensureListGrammar(node->identifier);
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
    LangData *langData;
    queue<ListNode>  unresolved;
    RegisterKeysVisitor(LangData *langData) : langData(langData) {}

    // Runs visitList until there are no more
    // lists in queue, or loop is detected.
    void visitSource(SourceNode *node) {
        DescrVisitor::visitSource(node);
        if (unresolved.size() > 0) {
            size_t prevSize = unresolved.size();
            int numTimesEqual = 0;
            while (unresolved.size() > 0) {
                visitList(unresolved.front());
                unresolved.pop();
                if (unresolved.size() == prevSize) {
                    ++numTimesEqual;
                    // Allow equal the same number of times
                    // as there are members in the queue.
                    if (numTimesEqual >= unresolved.size()) {
                        printf("Loop detected in lists\n");
                        for (ListNode *listNode : unresolved) {
                            printf("List: %s\n", listNode->identifier);
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
        } else if (typed2->type == PTOKEN {
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
}
class BuildRulesVisitor : public DescrVisitor {
public:
    LangData *langData;
    BuildLangDataVisitor(LangData *langData) : langData(langData) {}
    void visitEnum(EnumNode *node) {
        // Build rules
        string grammarKey = langData->keyFromTypeDecl(node->typeDecl);
        EnumGrammarType *grammarType = langData->ensureAstGrammar(grammarKey);
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
            if (astDef->identifier.compare("") != 0) {
                // Check for other ast rule
                if (langData->astGrammarTypes.count(astDef->identifier) != 0) {
                    // Get class name from other type
                    defClass = langData->astGrammarTypes[astDef]->identifier;
                }
            }
            GrammarRule *curRule = new GrammarRule();
            // Collect rule args
            vector<RuleArg> ruleArgs;
            int num = 0;
            vector<string> tokenList;
            for (AstPart *part : *astDef->nodes) {
                ++num;
                TypedPart *typedPart = langData->getTypedPart(part);
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
            curRule->action = new AstConstructionAction(
                defClass->identifier,
                ruleArgs
            );
            grammar->rules.push_back(curRule);
        }
    }
    void visitList(ListNode *node) {
        ListGrammarType *grammar = ListGrammarType();
        TypedPart *typed1 = langData->getTypedPart(node->astKey);
        TypedPart *typed2 = langData->getTypedPart(node->tokenSep);
        if (typed1 == nullptr ||typed2 == nullptr) {
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
        grammar->type = typedPart;
        // Init list
        GrammarRule *initRule = new GrammarRule();
        initRule->tokenList = new vector<string>;
        initRule->action = new ListInitAction(typedPart);
        grammar->rules->push_back(initRule);
        if (sepBetween) {
            GrammarRule *firstPart;
            firstPart->tokenList = new vector<string>{
                node->identifier,
                listType->identifier
            };
            firstPart->action = new ListPushAction(1, 2, listType);
            grammar->rules->push_back(firstPart);
            GrammarRule *sepPart;
            sepPart->tokenList = new vector<string>{
                node->identifier,
                sepToken->identifier,
                listType->identifier
            };
            sepPart->action = new ListPushAction(1, 3, listType);
            grammar->rules->push_back(sepPart);
        } else {
            GrammarRule *sepPart;
            sepPart->tokenList = new vector<string>{
                node->identifier,
                listType->identifier,
                sepToken->identifier
            };
            sepPart->action = new ListPushAction(1, 2, listType);
            grammar->rules->push_back(sepPart);
        }
    }
};

class BuildAstVisitor : public DescrVisitor {
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
        AstClass *baseAstClass = langData->ensureClass(baseAstName);
        // Go through rules and ensure ast classes
        // has needed members and constructors
        for (GrammarRule *rule : *grammar->rule) {
            AstConstructionAction *action = static_cast<AstConstructionAction>(rule->action);
            AstClass *ruleClass;
            if (action->astClass != baseAstClass) {
                ruleClass = langData->ensureClass(action->astClass);
                // Ensure subclass extends base class and
                // subclass registered as child
                if (ruleClass->extends != "" && ruleClass->extends != baseAstName) {
                    printf("Todo, handle different base");
                    exit(1);
                }
                ruleClass->extends = baseAstClass;
                baseAstClass->subClasses->insert(action->astClass);
            } else {
                ruleClass = baseAstClass;
            }
            // Ensure class has members for all args
            for (RuleArg *ruleArg : action->ruleArgs) {
                if (defClass->members.count(typedPart->identifier) != 0) {
                    if (defClass->members[typedPart->identifier]->type != typedPart) {
                        // Member has different type
                        printf("Member has different type");
                        exit(1);
                    }
                } else {
                    defClass->members.emplace(
                        typedPart->identifier,
                        AstClassMember(typedPart)
                    );
                }
            }
            // Ensure ast class constructor
            bool constructorFound = false;
            for (AstClassConstructor *constr : defClass->constructors) {
                if (constr->size() == action->ruleArgs.size()) {
                    // Do simple equality check.
                    // Concievably we could try to reorder, but
                    // that could also lead to more volatility
                    // in available constructors
                    // Perhaps when the ast interface is defined,
                    // reorder could be tried.
                    bool isEqual = true;
                    for (int i = 0; i < constr->size(); ++i) {
                        // Assume correspondance with member field types
                        if (*constr->args[i] != action->ruleArgs[i].identifier) {
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
                AstClassConstructor constr;
                for (RuleArg *ruleArg : action->ruleArgs) {
                    constr.args.push_back(action->ruleArg->identifier);
                }
                defClass->constructors.push_back(constr);
            }
        }
    }
};

}