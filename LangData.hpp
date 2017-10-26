#pragma once
#include "DescrNode.hpp"
#include "DescrVisitor.hpp"
#include <vector>
#include <map>
#include <set>

class TokenData {
public:
    TokenType type;
    std::string key;
    std::string regex;
    TokenData(TokenType type, std::string key, std::string regex)
        : type(type), key(key), regex(regex) {}
};

enum PartType {
    PSTRING,
    PINT,
    PFLOAT,
    PENUM,
    PAST,
    PLIST,
    PTOKEN
};
class TypedPart {
public:
    PartType type;
    std::string identifier;
    PartTypeBase(ParType type, std::string identifier) : type(type), identifier(identifier) {}
    bool operator==(const TypedPart &other) {
        return (other->type == this->type && other->identifier == this->identifier);
    }
    bool operator!=(const TypedPart &other) {
        return !(*this == other);
    }
};
class TypedPartToken {
public:
    TypedPartToken(std::string identifier)
        : TypedPart(PTOKEN, identifier) {}
};
class TypedPartPrim {
public:
    TypedPartPrim(PartType type, std::string identifier)
        : TypedPart(type, identifier) {}
};
class TypedPartEnum {
public:
    std::string enumName;
    TypedPartEnum(std::string identifier, std::string enumName) 
        : TypedPart(PENUM, identifier), enumName(enumName) {}
};
class TypedPartAst {
public:
    std::string astClass;
    TypedPartAst(std::string identifier, std::string astClass) 
        : TypedPart(PAST, identifier), astClass(astClass) {}
};
class TypedPartList {
public:
    // Little mismatch to represent all and nested types
    TypedPart *type;
    TypedPartList(std::string identifier, TypedPart *type) 
        : TypedPart(PLIST, identifier), type(type) {}
};

class RuleAction {};
class RuleArg {
public:
    int num;
    TypedPart *typedPart;
    RuleArg(int num, TypedPart *typePart) : num(num), typedPart(typedPart) {}
};

class AstConstruction : public RuleAction {
public:
    std::string astClass;
    std::vector<RuleArg> args;

};

class EnumValue : public RuleAction {
public:
    std::string enumName;
};

class ListInit : public RuleAction {
public:
    TypedPart *type;
    ListInit(TypedPart *type) : type(type) {}
};

class ListPush : public RuleAction {
public:
    int listNum;
    int elemNum;
    TypedPart *type;
    ListPush(int listNum, int elemNum, TypedPart *type)
        : listNum(listNum), elemNum(elemNum), type(type) {}
};

class GrammarRule {
public:
    std::vector<string> tokenList;
    RuleAction *action;
    GrammarRule() {}
};

class AstGrammarType {
public:
    std::string key;
    std::string astClass;
    std::vector<GrammarRule*> rules;
    AstGrammarType() {}
};

class ListGrammarType {
public:
    std::string key;
    // Little mismatch to represent all and nested types
    TypedPart *type;
    std::vector<GrammarRule*> rules;
    ListGrammarType() {}
};

class AstEnumMember {
public:
    std::string name;
    std::string string;
    AstEnumMember(std::string name, std::string string) 
        : name(name), string(string) {}
};

class AstEnum {
public:
    std::string name;
    std::set<std::string> members;
    AstEnum(std::string name) : name(name) {}
};

class AstClassMember {
public:
    TypedPart *typedPart;
    AstClassMember(TypedPart *typedPart) : typedPart(typedPart) {}
};

class AstClassConstructor {
public:
    std::vector<std::string> args;
};

class AstClass {
public:
    std::string identifier;
    std::string extends;
    std::map<std::string, AstClassMember*> members;
    std::vector<AstClassConstructor*> constructors;
    std::set<std::string> subClasses;
    AstClass() {}
    void ensureMember(TypedPart *typedPart) {
        if (members.count(typedPart->identifier) == 0) {
            members.emplace(typedPart->identifier, new AstClassMember(typedPart));
        } else {
            // Verify type
        }
    }
};

class LangData {
public:
    std::map<std::string, TokenData*> tokenData;
    std::map<std::string, AstEnum*> enums;
    std::set<TokenType> tokenTypes;
    std::map<std::string, AstGrammarType> astGrammarTypes;
    std::map<std::string, ListGrammarType> listGrammarTypes;
    std::map<std::string, AstClass> astClasses;
    LangData() {
    }
    AstClass* ensureClass(std::string className) {
        if (astClasses.count(className) == 0) {
            astClasses.emplace(className, AstClass());
        }
        return &astClasses[className];
    }
};

class BuildLangDataVisitor : public DescrVisitor {
public:
    LangData *langData;
    std::string curAstBaseType;
    GrammarType *curGrammarType;
    BuildLangDataVisitor(LangData *langData) : langData(langData) {}
    void visitToken(TokenNode *node) {
        langData->tokenData.emplace(node->identifier, new TokenData(
            node->type,
            node->identifier,
            node->regex
        ));
        if (node->type != NONE) {
            langData->tokenTypes.insert(node->type);
        }
    }
    void visitEnumDecl(EnumDeclNode *node) {
        langData->tokenData.emplace(node->identifier, new TokenData(
            NONE,
            node->identifier,
            node->regex
        ));
    }
    void visitEnum(EnumNode *node) {
        if (!langData->enums.count(node->typeDecl->identifier)) {
            langData->enums.emplace(node->typeDecl->identifier, new AstEnum(node->typeDecl->identifier));
        }
        AstEnum* astEnum = langData->enums[node->typeDecl->identifier];
        for (EnumDeclNode *enumDecl : *node->nodes) {
            astEnum->members.insert(enumDecl->identifier);
        }
        DescrVisitor::visitEnum(node);
    }
    TypedPart* getTypedPart(AstPart *astPart) {
        // Check token
        if (langData->tokenData.count(part->identifier) != 0) {
            TokenData *tokenRef = langData->tokenData[part->identifier];
            if (tokenRef->type == NONE) {
                // Ignore const literal token
                return NULL;
            }
            switch (tokenRef->type) {
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
        } else if (langData->enums.count(part->identifier) != 0) {
            AstEnum *enumData = langData->enums[part->identifier];
            return new TypedPartEnum(part->identifier, enumData->name);
        } else if (langData->astGrammarTypes.count(part->identifier) != 0) {
            return new TypedPartAst(
                part->identifier,
                langData->astGrammarTypes[part->identifier].astClass
            )
        } else if (langData->listGrammarTypes.count(part->identifier) != 0) {
            return new TypedPartList(
                part->identifier,
                langData->listGrammarTypes[part->identifier]->type
            )
        } else if (langData->tokenData.count(part->identifier) != 0) {
            return new TypedPartToken(part->identifier);
        } else {
            return NULL;
        }
    }

    void visitAst(AstNode *node) {
        // Key is given as alias or use class name
        std::string grammarKey = (node->typeDecl->alias.compare("") == 0) ?
            node->typeDecl->identifier : node->typeDecl->alias;
        // Get current grammar
        if (langData->astGrammarTypes.count(grammarKey) == 0) {
            langData->astGrammarTypes->emplace(grammarKey, GrammarType());
        }
        curGrammarType = &langData->astGrammarTypes[grammarKey];
        // Get ast base class
        curAstBaseType = node->typeDecl->identifier;
        if (langData->astClasses.count(curAstBaseType) == 0) {
            langData->astClasses.emplace(curAstBaseType, AstClass());
        }
        AstClass *baseAstClass = &langData->astClasses[curAstBaseType];
        // Go through defs and collect rules, ensure ast classes
        // has needed members and constructors
        for (AstDef* astDef : *node->nodes) {
            AstClass *defClass = NULL;
            // Defs may have an identifier, this would refer
            // to a subclass of the baseClass or another ast
            // rule.
            if (astDef->identifier.compare("") != 0) {
                // Check for other ast rule
                if (langData->astGrammarTypes.count(astDef->identifier) != 0) {
                    // Ensure referred rules class is registered
                    // as subclass of base class.
                    baseAstClass->subClasses.insert(
                        langData->astGrammarTypes[astDef->identifier].className
                    );
                    // Rest should be handled when processing
                    // referred rule.
                    continue;
                }
                // We have (probably) a subclass
                if (astDef->identifier.compare(baseAstClass->identifier) != 0) {
                    defClass = langData->ensureClass(astDef->identifier);
                } else {
                    defClass = baseAstClass;
                }
            } else {
                defClass = baseAstClass;
            }
            GrammarRule *curRule = new GrammarRule();
            // Collect constructor args
            std::vector<RuleArg> ruleArgs;
            int num = 0;
            std::vector<std::string> tokenList;
            for (AstPart *part : *astDef->nodes) {
                ++num;
                TypedPart *typedPart = getTypedPart(part);
                tokenList.push_back(typedPart->identifier);
                if (typedPart->type == PTOKEN || typedPart == nullptr) {
                    // Skip const literal tokens
                    continue;
                }
                // Ensure ast class member
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
                // Add rule arg
                ruleArgs.push_back(RuleArg(num, typedPart));
            }
            // Ensure ast class constructor
            bool constructorFound = false;
            for (AstClassConstructor *constr : *defClass->constructors) {
                if (constr->size() == ruleArgs.size()) {
                    // Do simple equality check.
                    // Concievably we could try to reorder, but
                    // that could also lead to more volatility
                    // in available constructors
                    // Perhaps when the ast interface is defined,
                    // reorder could be tried.
                    bool isEqual = true;
                    for (int i = 0; i < constr->size(); ++i) {
                        if (*constr->args[i]->typedPart != ruleArgs[i].typedPart) {
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
                for (RuleArg *ruleArg : ruleArgs) {
                    constr.args.push_back(ruleArg->identifier);
                }
                defClass->constructors.push_back(constr);
            }
            // Add rule to grammar type
            curRule->tokenList = tokenList;
            curRule->action = new AstConstruction(
                defClass->identifier,
                ruleArgs
            );
            curGrammarType->rules.push_back(curRule);
        }
    }
    void visitList(ListNode *node) {
        ListGrammarType *grammarType = ListGrammarType();
        TypedPart *typed1 = getTypedPart(node->astKey);
        TypedPart *typed2 = getTypedPart(node->tokenSep);
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
        grammarType->type = typedPart;
        // Init list
        GrammarRule *initRule = new GrammarRule();
        initRule->tokenList = new std::vector<std::string>;
        initRule->action = new ListInit(typedPart);
        grammarType->rules->push_back(initRule);
        if (sepBetween) {
            GrammarRule *firstPart;
            firstPart->tokenList = new std::vector<std::string>{
                node->identifier,
                listType->identifier
            };
            firstPart->action = new ListPush(1, 2, listType);
            grammarType->rules->push_back(firstPart);
            GrammarRule *sepPart;
            sepPart->tokenList = new std::vector<std::string>{
                node->identifier,
                sepToken->identifier,
                listType->identifier
            };
            sepPart->action = new ListPush(1, 3, listType);
            grammarType->rules->push_back(sepPart);
        } else {
            GrammarRule *sepPart;
            sepPart->tokenList = new std::vector<std::string>{
                node->identifier,
                listType->identifier,
                sepToken->identifier
            };
            sepPart->action = new ListPush(1, 2, listType);
            grammarType->rules->push_back(sepPart);
        }
    }
};