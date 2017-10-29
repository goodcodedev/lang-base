#pragma once
#include "DescrNode.hpp"
#include "DescrVisitor.hpp"
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <iostream>
#include <fstream>
#include <array>
#include <cctype>

extern FILE *yyin;
extern int yyparse();
extern DescrNode *result; 

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
    string getGrammarToken() {
        return key + "_T";
    }
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

class LData;

/**
 * Represents a visitor case in generated ToSourceVisitor
 * Needs to capture different cases and
 * possibly act differently on variations.
 * So there can be several visitors (one for each case)
 * for an ast class todo
 * Possibly solve this by registering
 * a rule identifier (serialized token list), or checking for default valued
 * fields. Possibly this would need more token values
 * to be captured.
 * Another approach is to systematically branch
 * on rule tokens.
 */
class ToSourceVisitor {
public:
    string astClass;
    LData *langData;
    string code;
    ToSourceVisitor(string astClass, LData *langData)
        : astClass(astClass), langData(langData) {}
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
    virtual void generateGrammarVal(string *str, int num, LData *langData) = 0;
    virtual void generateGrammarType(string *str, LData *langData) = 0;
    string getGrammarToken() {
        switch (type) {
            case PSTRING:
            case PINT:
            case PFLOAT:
            case PTOKEN:
            return identifier + "_T";
            break;
            default:
            return identifier;
        }
    }
    string getMemberKey() {
        if (alias != "" && alias != identifier) {
            return alias;
        } else if (type == PENUM || type == PAST) {
            // Lowercase
            string lowercaseFirst = identifier;
            lowercaseFirst[0] = std::tolower(lowercaseFirst[0]);
            return lowercaseFirst;
        } else {
            return identifier;
        }
    }
    virtual void addToVisitor(ToSourceVisitor *visitor) = 0;
};

// Token part
class TypedPartToken : public TypedPart {
public:
    TypedPartToken(string identifier)
        : TypedPart(PTOKEN, identifier) {}
    void generateGrammarVal(string *str, int num, LData *langData);
    void generateGrammarType(string *str, LData *langData);
    void addToVisitor(ToSourceVisitor *visitor);
};
// Prim token part
class TypedPartPrim : public TypedPart {
public:
    TypedPartPrim(PartType type, string identifier)
        : TypedPart(type, identifier) {}
    void generateGrammarVal(string *str, int num, LData *langData);
    void generateGrammarType(string *str, LData *langData);
    void addToVisitor(ToSourceVisitor *visitor);
};
// Enum part
// These are stored as integers
class TypedPartEnum : public TypedPart {
public:
    string enumKey;
    TypedPartEnum(string identifier, string enumKey) 
        : TypedPart(PENUM, identifier), enumKey(enumKey) {}
    void generateGrammarVal(string *str, int num, LData *langData);
    void generateGrammarType(string *str, LData *langData);
    void addToVisitor(ToSourceVisitor *visitor);
};
// Ast part
class TypedPartAst : public TypedPart {
public:
    string astClass;
    TypedPartAst(string identifier, string astClass) 
        : TypedPart(PAST, identifier), astClass(astClass) {}
    void generateGrammarVal(string *str, int num, LData *langData);
    void generateGrammarType(string *str, LData *langData);
    void addToVisitor(ToSourceVisitor *visitor);
};
// List part
class TypedPartList : public TypedPart {
public:
    // Little mismatch to represent all and nested types
    TypedPart *type;
    TypedPart *sep;
    bool sepBetween;
    TypedPartList(string identifier, TypedPart *type, TypedPart *sep, bool sepBetween) 
        : TypedPart(PLIST, identifier), type(type), sep(sep), sepBetween(sepBetween) {}
    void generateGrammarVal(string *str, int num, LData *langData);
    void generateGrammarType(string *str, LData *langData);
    void addToVisitor(ToSourceVisitor *visitor);
};

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
    AstConstructionAction(string astClass) : RuleAction(RAAstConstruction), astClass(astClass)  {}
    AstConstructionAction(string astClass, vector<RuleArg> args)
        : RuleAction(RAAstConstruction), astClass(astClass), args(args) {}
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
    TypedPart *type;
    ListPushAction(int listNum, int elemNum, TypedPart *type)
        : RuleAction(RAListPush), listNum(listNum), elemNum(elemNum), type(type) {}
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

class AstClass;
/**
 * Represents an enum in code
 */
class AstEnum {
public:
    string name;
    set<string> members;
    map<string, string> values;
    AstEnum(string name) : name(name) {}
    void generateDefinition(string *str, LData *langData);
    void generateToStringMethod(string *str, LData *langData);
};

/**
 * Represents an ast class member in code
 */
class AstClassMember {
public:
    TypedPart *typedPart;
    AstClassMember(TypedPart *typedPart) : typedPart(typedPart) {}
    void generateMember(string *str, LData *langData, AstClass *astClass);
};

/**
 * Represent an ast class constructor in code
 */
class AstClassConstructor {
public:
    vector<string> args;
    AstClassConstructor() {}
    void generateConstructor(string *str, LData *langData, AstClass *astClass);
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
    void generateHeader(string *str, LData *langData);
    void generateDefinition(string *str, LData *langData);
};

/**
 * Central object for lang data.
 */
class LData {
public:
    string langKey;
    map<string, TokenData*> tokenData;
    map<string, AstEnum*> enums;
    set<TokenType> tokenTypes;
    map<string, AstGrammarType*> astGrammarTypes;
    map<string, ListGrammarType*> listGrammarTypes;
    map<string, EnumGrammarType*> enumGrammarTypes;
    map<string, AstClass*> astClasses;
    string startKey;
    StartAction *startAction;
    LData(string langKey) : langKey(langKey) {}
    // Some built in tokens provided
    // from identifier
    // Returns nullptr when not found.
    TokenData* getBuiltInToken(string identifier);
    // Add built in token or fail
    void addBuiltInToken(string identifier);
    AstClass* ensureClass(string className);
    AstEnum* ensureEnum(string typeName);
    EnumGrammarType* ensureEnumGrammar(string key);
    AstGrammarType* ensureAstGrammar(string key);
    ListGrammarType* ensureListGrammar(string key);
    // Ensures sub relationsship, and returns the subclass.
    // Will check for equality and just return base class
    // if equal.
    AstClass* ensureSubRelation(string baseClass, string subClass);
    TypedPart* getTypedPart(string identifier);
    string keyFromTypeDecl(TypeDecl *typeDecl);
};

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
void TypedPartToken::addToVisitor(ToSourceVisitor *visitor) {
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
void TypedPartPrim::addToVisitor(ToSourceVisitor *visitor) {
    switch (type) {
        case PSTRING:
        visitor->code += "str += node->" + getMemberKey() + ";\n";
        break;
        case PINT:
        visitor->code += "str += std::to_string(node->" + getMemberKey() + ");\n";
        break;
        case PFLOAT:
        visitor->code += "str += std::to_string(node->" + getMemberKey() + ");\n";
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
void TypedPartEnum::addToVisitor(ToSourceVisitor *visitor) {
    visitor->code += "str += enum" + identifier + "ToString(node->" + getMemberKey() + ");\n";
}

void TypedPartAst::generateGrammarVal(string *str, int num, LData *langData) {
    *str += "reinterpret_cast<" + astClass + "*>($" + std::to_string(num) + ")";
}
void TypedPartAst::generateGrammarType(string *str, LData *langData) {
    *str += astClass + "*";
}
void TypedPartAst::addToVisitor(ToSourceVisitor *visitor) {
    visitor->code += "visit" + astClass + "(node->" + getMemberKey() + ");\n";
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
void TypedPartList::addToVisitor(ToSourceVisitor *visitor) {
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
        printf("Currently only ast supported");
        exit(1);
    }
}

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

TokenData* LData::getBuiltInToken(string identifier) {
    if (identifier == "LPAREN") return new TokenData(NONE, "LPAREN", "\\(");
    if (identifier == "RPAREN") return new TokenData(NONE, "RPAREN", "\\)");
    if (identifier == "LBRACE") return new TokenData(NONE, "LBRACE", "\\{");
    if (identifier == "RBRACE") return new TokenData(NONE, "RBRACE", "\\}");
    if (identifier == "COMMA") return new TokenData(NONE, "COMMA", "\\,");
    if (identifier == "SEMICOLON") return new TokenData(NONE, "SEMICOLON", "\\;");
    if (identifier == "EQUAL") return new TokenData(NONE, "EQUAL", "\\=");
    if (identifier == "intConst") return new TokenData(TINT, "intConst", "[1-9][0-9]*");
    if (identifier == "identifier") return new TokenData(TSTRING, "identifier", "[_a-zA-Z][0-9_a-zA-Z]*");
    // Whitespace token
    // Not added to rules currently
    // Used when whitespace is needed in ToString source
    if (identifier == "WS") return new TokenData(NONE, "WS", " ");
    return nullptr;
}
// Add built in token or fail
void LData::addBuiltInToken(string identifier) {
    // Check for built in
    TokenData *builtIn = getBuiltInToken(identifier);
    if (builtIn == nullptr) {
        printf("Key not found: %s\n", identifier.c_str());
        exit(1);
    }
    // Add built in token
    tokenData.emplace(identifier, builtIn);
    if (builtIn->type != NONE) {
        tokenTypes.insert(builtIn->type);
    }
}
AstClass* LData::ensureClass(string className) {
    if (astClasses.count(className) == 0) astClasses.emplace(className, new AstClass(className));
    return astClasses[className];
}
AstEnum* LData::ensureEnum(string typeName) {
    if (!enums.count(typeName)) enums.emplace(typeName, new AstEnum(typeName));
    return enums[typeName];
}
EnumGrammarType* LData::ensureEnumGrammar(string key) {
    if (!enumGrammarTypes.count(key)) enumGrammarTypes.emplace(key, new EnumGrammarType(key));
    return enumGrammarTypes[key];
}
AstGrammarType* LData::ensureAstGrammar(string key) {
    if (!astGrammarTypes.count(key)) astGrammarTypes.emplace(key, new AstGrammarType(key));
    return astGrammarTypes[key];
}
ListGrammarType* LData::ensureListGrammar(string key) {
    if (!listGrammarTypes.count(key)) listGrammarTypes.emplace(key, new ListGrammarType(key));
    return listGrammarTypes[key];
}
// Ensures sub relationsship, and returns the subclass.
// Will check for equality and just return base class
// if equal.
AstClass* LData::ensureSubRelation(string baseClass, string subClass) {
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
TypedPart* LData::getTypedPart(string identifier) {
    // Check token
    if (tokenData.count(identifier) != 0) {
        TokenData *tokenRef = tokenData[identifier];
        switch (tokenRef->type) {
            case NONE:
            return new TypedPartToken(tokenRef->key);
            break;
            case TSTRING:
            return new TypedPartPrim(PSTRING, tokenRef->key);
            break;
            case TINT:
            return new TypedPartPrim(PINT, tokenRef->key);
            break;
            case TFLOAT:
            return new TypedPartPrim(PFLOAT, tokenRef->key);
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
            listGrammarTypes[identifier]->type,
            listGrammarTypes[identifier]->sep,
            listGrammarTypes[identifier]->sepBetween
        );
    } else if (tokenData.count(identifier) != 0) {
        return new TypedPartToken(identifier);
    } else {
        return nullptr;
    }
}
string LData::keyFromTypeDecl(TypeDecl *typeDecl) {
    return (typeDecl->alias.compare("") != 0) ? typeDecl->alias : typeDecl->identifier;
}

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
// Adds built in tokens.
// Keys should be registered as they can
// override built in tokens.
class AddBuiltInTokens : public DescrVisitor {
public:
    LData *langData;
    AddBuiltInTokens(LData *langData) : langData(langData) {}
    void visitAstPart(AstPart *node) {
        TypedPart *typed = langData->getTypedPart(node->identifier);
        if (typed == nullptr) {
            langData->addBuiltInToken(node->identifier);
        }
    }
    void visitList(ListNode *node) {
        // Simply check for null on either
        TypedPart *typed1 = langData->getTypedPart(node->astKey);
        if (typed1 == nullptr) langData->addBuiltInToken(node->astKey);
        TypedPart *typed2 = langData->getTypedPart(node->tokenSep);
        if (typed2 == nullptr) langData->addBuiltInToken(node->tokenSep);
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
        grammarType->sep = sepToken;
        grammarType->sepBetween = sepBetween;
    }
};
// Builds rules
// Also sets up start node
class BuildRulesVisitor : public DescrVisitor {
public:
    LData *langData;
    BuildRulesVisitor(LData *langData) : langData(langData) {}
    void visitStart(StartNode *node) {
        langData->startKey = node->identifier;
        langData->startAction = new StartAction(langData->getTypedPart(langData->startKey));
    }
    void visitEnum(EnumNode *node) {
        // Build rules
        string grammarKey = langData->keyFromTypeDecl(node->typeDecl);
        EnumGrammarType *grammarType = langData->ensureEnumGrammar(grammarKey);
        // Enums are one of several token values
        for (EnumDeclNode *enumDecl : *node->nodes) {
            GrammarRule *rule = new GrammarRule();
            // Use typed part to get possibly
            // changed identifier
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
                    TypedPart *typedPart = langData->getTypedPart(part->identifier);
                    // Skip WS (whitespace) token as this is ignored in grammar
                    if (typedPart->type == PTOKEN && typedPart->identifier == "WS") continue;
                    ++num;
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
        TypedPart *listType = grammar->type;
        TypedPart *sepToken = grammar->sep;
        bool sepBetween = grammar->sepBetween;
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
            astEnum->values.emplace(enumDecl->identifier, enumDecl->regex);
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
                string memberKey = typedPart->getMemberKey();
                if (ruleClass->members.count(memberKey) != 0) {
                    // This equality test will check identifier, alias and type
                    // It should probably test type more thoroughly, and
                    // rather check resolved key todo
                    if (*ruleClass->members[memberKey]->typedPart != *typedPart) {
                        // Member has different type
                        printf("Member has different type");
                        exit(1);
                    }
                } else {
                    ruleClass->members.emplace(
                        memberKey,
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
                        if (constr->args[i] != action->args[i].typedPart->getMemberKey()) {
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
                    constr->args.push_back(ruleArg.typedPart->getMemberKey());
                }
                ruleClass->constructors.push_back(constr);
            }
        }
    }
};

/**
 * Generates toSource visitor for language
 */
class ToSourceGenVisitor : public DescrVisitor {
public:
    LData *langData;
    map<string, ToSourceVisitor*> visitors;
    ToSourceGenVisitor(LData *langData)
        : langData(langData) {}
    ToSourceVisitor* getToSourceVisitor(string astClass) {
        if (visitors.count(astClass) == 0) {
            visitors.emplace(astClass, new ToSourceVisitor(astClass, langData));
        }
        return visitors[astClass];
    }
    void visitAst(AstNode *node) {
        string baseClass = node->typeDecl->identifier;
        for (AstDef *def : *node->nodes) {
            string defClass = baseClass;
            // Identifier can refer to other ast node
            if (def->identifier != "") {
                TypedPart *idPart = langData->getTypedPart(def->identifier);
                if (idPart != nullptr && idPart->type == PAST) {
                    // Let other ast visit handle this
                    continue;
                } else {
                    defClass = def->identifier;
                }
            }
            ToSourceVisitor *visitor = getToSourceVisitor(defClass);
            for (AstPart *defPart : *def->nodes) {
                TypedPart *part = langData->getTypedPart(defPart->identifier);
                part->addToVisitor(visitor);
            }
        }
    }
};

class SourceGenerator {
public:
    LData *langData;
    string folder;
    SourceGenerator(LData *langData, string folder) 
        : langData(langData), folder(folder) {}

    void saveToFile(string *str, string fileName) {
        std::ofstream f;
        f.open(folder + "/" + fileName);
        f << *str;
        printf("%s", str->c_str());
        f.close();
    }
    // Generate flex file
    void generateLexFile() {
        string str = "";
        str +=  "%{\n";
        str += "#include \"" + langData->langKey + ".tab.h\"\n";
        str +=  "#define register // Deprecated in c++11\n"
                "#ifdef _WIN32\n"
                "   #define __strdup _strdup\n"
                "#else\n"
                "   #define __strdup strdup\n"
                "#endif\n"
                "%}\n"
                "%option yylineno\n"
                "%%\n";
        for (auto const &pair : langData->tokenData) {
            TokenData *token = pair.second;
            if (token->key == "WS") {
                // Used to signal whitespace needed
                // in source code
                continue;
            }
            switch (token->type) {
                case NONE:
                str += token->regex + " { return " + token->getGrammarToken() + "; }\n";
                break;
                case TINT:
                str += token->regex + " { yylval.ival = atoi(yytext); return " + token->getGrammarToken() + "; }\n";
                break;
                case TSTRING:
                str += token->regex + " { yylval.sval = __strdup(yytext); return " + token->getGrammarToken() + "; }\n";
                break;
                case TFLOAT:
                str += token->regex + " { yylval.fval = atof(yytext); return " + token->getGrammarToken() + "; }\n";
                break;
            }
        }
        str +=  "%%\n"
                "int yywrap() { return 1; }\n";
        saveToFile(&str, "gen/" + langData->langKey + ".l");
    }
    // Generate bison grammar
    void generateGrammarFile() {
        string str = "";
        str +=  "%{\n"
                "#include <stdio.h>\n"
                "#include \"" + langData->langKey + ".hpp\"\n";
        // Result variable
        if (langData->startAction == nullptr) {
            printf("Need start key\n");
            exit(1);
        }
        langData->startAction->startPart->generateGrammarType(&str, langData);
        str += " result;\n";
        str +=  "extern FILE *yyin;\n"
                "void yyerror(const char *s);\n"
                "extern int yylex(void);\n"
                "extern int yylineno;\n"
                "%}\n";
        // Union
        str +=  "%union {\n"
                "   void *ptr;\n";
        for (TokenType ttype : langData->tokenTypes) {
            switch (ttype) {
                case TINT: str += "    int ival;\n"; break;
                case TSTRING: str += "    char *sval;\n"; break;
                case TFLOAT: str += "    double fval;\n"; break;
                case NONE: break;
            }
        }
        str += "}\n";
        // Tokens
        for (auto const &pair : langData->tokenData) {
            TokenData *token = pair.second;
            if (token->key == "WS") {
                // Used to signal whitespace needed
                // in source code
                continue;
            }
            switch (token->type) {
                case NONE: str += "%token " + token->getGrammarToken() + "\n"; break;
                case TINT: str += "%token <ival> " + token->getGrammarToken() + "\n"; break;
                case TSTRING: str += "%token <sval> " + token->getGrammarToken() + "\n"; break;
                case TFLOAT: str += "%token <fval> " + token->getGrammarToken() + "\n"; break;
            }
        }
        // Types
        // Enums goes to ival
        if (langData->enumGrammarTypes.size() > 0) {
            str += "%type <ival> ";
            for (auto const &pair : langData->enumGrammarTypes) {
                EnumGrammarType *enumGrammar = pair.second;
                str += enumGrammar->key + " ";
            }
            str += "\n";
        }
        // Ast and list goes to ptr
        if (langData->astGrammarTypes.size() > 0 || langData->listGrammarTypes.size() > 0) {
            str += "%type <ptr> ";
            str += "start ";
            for (auto const &pair : langData->astGrammarTypes) {
                AstGrammarType *astGrammar = pair.second;
                str += astGrammar->key + " ";
            }
            for (auto const &pair : langData->listGrammarTypes) {
                ListGrammarType *listGrammar = pair.second;
                str += listGrammar->key + " ";
            }
            str += "\n";
        }
        str += "%%\n";
        // Add rules
        // Start rule first
        str += "start: " + langData->startKey + " { ";
        langData->startAction->generateGrammar(&str, langData);
        str += " }\n    ;\n";
        // Ast types
        for (auto const &grammar : langData->astGrammarTypes) {
            grammar.second->generateGrammar(&str, langData);
        }
        // List types
        for (auto const &grammar : langData->listGrammarTypes) {
            grammar.second->generateGrammar(&str, langData);
        }
        // Enum types
        for (auto const &grammar : langData->enumGrammarTypes) {
            grammar.second->generateGrammar(&str, langData);
        }
        str += "\n%%\n";

        str += "void yyerror(const char *s) {\n"
               "    printf(\"Parse error on line %d: %s\", yylineno, s);\n"
               "}\n";
        saveToFile(&str, "gen/" + langData->langKey + ".y");
    }

    // Recursive to ensure parent classes
    // are added before extending classes
    void generateHeaderClass(string *str, string astClass, set<string> *addedClasses) {
        if (addedClasses->find(astClass) != addedClasses->end()) return;
        AstClass *cls = langData->astClasses[astClass];
        if (cls->extends != "") {
            generateHeaderClass(str, cls->extends, addedClasses);
        }
        cls->generateHeader(str, langData);
        addedClasses->insert(astClass);
    }

    // Generate c++ classes, enums etc
    void generateAstClasses() {
        string str = "#pragma once\n";
        str += "#include <string>\n";
        str += "#include <vector>\n";
        // Create enum with entries for each class
        str +=  "enum NodeType {\n    ";
        bool isFirst = true;
        for (auto const &astClass : langData->astClasses) {
            if (!isFirst) str += ", ";
            str += astClass.second->identifier + "Node";
            isFirst = false;
        }
        str += "\n};\n";
        for (auto const &astEnum : langData->enums) {
            astEnum.second->generateDefinition(&str, langData);
        }
        for (auto const &astEnum : langData->enums) {
            astEnum.second->generateToStringMethod(&str, langData);
        }
        // AstNode base class with nodeType
        str +=  "class AstNode {\n"
                "public:\n"
                "    NodeType nodeType;\n"
                "    AstNode(NodeType nodeType) : nodeType(nodeType) {}\n"
                "    virtual ~AstNode() {}\n"
                "};\n";
        // Forward declare classes
        for (auto const &astClass : langData->astClasses) {
            str += "class " + astClass.first + ";\n";
        }
        // Recursive method to ensure parent classes
        // are added before subclasses.
        set<string> addedClasses;
        for (auto const &astClass : langData->astClasses) {
            generateHeaderClass(&str, astClass.first, &addedClasses);
        }
        // Some externs, needed for parseFile
        str += "extern FILE *yyin;\n";
        str += "extern int yyparse();\n";
        // This extern requires ast header
        str += "extern ";
        langData->startAction->startPart->generateGrammarType(&str, langData);
        str += " result;\n";
        str += "class Loader {\npublic:\n";
        str += "static ";
        langData->startAction->startPart->generateGrammarType(&str, langData);
        str += " parseFile(std::string fileName) {\n";
        str +=  "   FILE *sourceFile;\n"
                "   #ifdef _WIN32\n"
                "   fopen_s(&sourceFile, fileName.c_str(), \"r\");\n"
                "   #else\n"
                "   sourceFile = fopen(fileName.c_str(), \"r\");\n"
                "   #endif\n"
                "   if (!sourceFile) {\n"
                "       printf(\"Can't open file %s\", fileName.c_str());\n"
                "       exit(1);\n"
                "   }\n"
                "   yyin = sourceFile;\n"
                "   do {\n"
                "       yyparse();\n"
                "   } while (!feof(yyin));\n"
                "   return result;\n"
                "}\n";
        str += "};\n";
        saveToFile(&str, "gen/" + langData->langKey + ".hpp");
    }
    void generateVisitor() {
        string *str = new string;
        *str += "#include \"" + langData->langKey + ".hpp\"\n";
        string className = langData->langKey + "Visitor";
        // Generate declaration
        *str += "class " + className + " {\n";
        *str += "public:\n";
        for (auto const &astClass : langData->astClasses) {
            *str += "   virtual void visit" + astClass.second->identifier + "(";
            *str += astClass.second->identifier + " *node);\n";
        }
        *str += "};\n";
        // Generate definitions
        for (auto const &astClass : langData->astClasses) {
            *str += "void " + className + "::visit" + astClass.second->identifier + "(";
            *str += astClass.second->identifier + " *node) {\n";
            // If this class has subclasses, pass on to more specific
            // visitor.
            // It's possibly nice to handle common members here,
            // but this would require every subclass to be passed
            // to this visitor for consistency.
            if (astClass.second->subClasses.size() > 0) {
                *str += "    switch(node->nodeType) {\n";
                for (string subClass : astClass.second->subClasses) {
                    *str += "        case " + subClass + "Node: ";
                    *str += "visit" + subClass + "(static_cast<" + subClass + "*>(node));break;\n";
                }
                *str += "        default:break;\n";
                *str += "    }\n";
            } else {
                for (auto const &member : astClass.second->members) {
                    switch (member.second->typedPart->type) {
                        case PAST: {
                            TypedPartAst *astPart = static_cast<TypedPartAst*>(member.second->typedPart);
                            AstClass *memberClass = langData->astClasses[astPart->astClass];
                            *str += "    visit" + memberClass->identifier + "(node->" + member.first + ");\n";
                        }
                        break;
                        case PLIST: {
                            // Todo list of lists
                            TypedPartList *listType = static_cast<TypedPartList*>(member.second->typedPart);
                            // Loop list of ast elements, then
                            // based on it's node type, cast it and
                            // pass to it's visitor
                            if (listType->type->type == PAST) {
                                TypedPartAst *listAstPart = static_cast<TypedPartAst*>(listType->type);
                                AstClass *listAstClass = langData->astClasses[listAstPart->astClass];
                                *str += "    for (";
                                listType->type->generateGrammarType(str, langData);
                                *str += " elem : *node->" + member.first + ") {\n";
                                *str += "        visit" + listAstClass->identifier + "(elem);\n";
                                *str += "    }\n";
                            }
                        }
                        break;
                        default:
                        break;
                    }
                }
            }
            *str += "}\n";
        }
        saveToFile(str, "gen/" + langData->langKey + "Visitor.hpp");
    }
    void generateToSource(SourceNode *source) {
        string *str = new string;
        *str += "#include \"" + langData->langKey + "Visitor.hpp\"\n";
        *str += "#include <string>\n";
        *str += "class " + langData->langKey + "ToSource ";
        *str += ": public " + langData->langKey + "Visitor {\n";
        *str += "public:\n";
        *str += "std::string str;\n";
        ToSourceGenVisitor sourceGenVisitor = ToSourceGenVisitor(langData);
        sourceGenVisitor.visitSource(source);
        for (auto const &visitor : sourceGenVisitor.visitors) {
            *str += "void visit" + visitor.first + "(" + visitor.first + "* node) {\n";
            *str += visitor.second->code;
            *str += "}\n";
        }
        *str += "};\n";
        saveToFile(str, "gen/" + langData->langKey + "ToSource.hpp");
    }
    void generateTransformer(){}

    /**
     * Executes command and return output
     * Returns nullptr on error status
     */
    string* execute(string cmd) {
        std::array<char, 128> buffer;
        string *result = new string;
        FILE* pipe(popen(cmd.c_str(), "r"));
        if (!pipe) {
            printf("Pipe failed");
            exit(1);
        }
        while (!feof(pipe)) {
            if (fgets(buffer.data(), 128, pipe) != nullptr)
                *result += buffer.data();
        }
        pclose(pipe);
        printf("cmd: %s\n%s\n", cmd.c_str(), result->c_str());
        return result;
    }

    void runFlexBison() {
        // Flex
        string lexFile = folder + "/gen/" + langData->langKey + ".l"; 
        string lexOutput = folder + "/gen/" + langData->langKey + ".yy.cpp"; 
        string *lexResult = execute("flex -o " + lexOutput + " " + lexFile);
        // Bison
        string grammarFile = folder + "/gen/" + langData->langKey + ".y"; 
        string grammarOutput = folder + "/gen/" + langData->langKey + ".tab.h"; 
        string grammarHeader = folder + "/gen/" + langData->langKey + ".tab.cpp"; 
        string *grammarResult = execute("bison -o " + grammarOutput
            + " --defines=" + grammarHeader + " " + grammarFile);
    }
};


SourceNode* parseDescr(string fileName) {
    FILE *sourceFile;
    errno = 0;
#ifdef _WIN32
    fopen_s(&sourceFile, fileName.c_str(), "r");
#else
    sourceFile = fopen(fileName.c_str(), "r");
#endif
	if (!sourceFile) {
		printf("Can't open file err:%d, file:%s\n", errno, fileName.c_str());
		exit(1);
	}
	yyin = sourceFile;
	do {    
		yyparse();
	} while (!feof(yyin));
	//std::string str; 
	//result->toStringF(&str, new FormatState());
	//fprintf(stdout, str.c_str());
	return static_cast<SourceNode*>(result);
}

/**
 * Runs the pipeline to generate files
 */
void genFiles(string folder, string langKey) {
	auto result = parseDescr(folder + "/" + langKey + ".lang");
	auto langData = new LData(langKey);
	auto keysVisit = new RegisterKeysVisitor(langData);
	auto listVisit = new RegisterListKeysVisitor(langData);
	auto builtInVisit = new AddBuiltInTokens(langData);
	auto rulesVisit = new BuildRulesVisitor(langData);
	auto astVisit = new BuildAstVisitor(langData);
	keysVisit->visitSource(result);
	builtInVisit->visitSource(result);
	listVisit->visitSource(result);
	rulesVisit->visitSource(result);
    astVisit->visitSource(result);
	SourceGenerator *sourceGen = new SourceGenerator(langData, folder);
    sourceGen->execute("mkdir -p " + folder + "/gen");
	sourceGen->generateLexFile();
	sourceGen->generateGrammarFile();
	sourceGen->generateAstClasses();
    sourceGen->generateVisitor();
    sourceGen->generateToSource(result);
	sourceGen->generateTransformer();
	sourceGen->runFlexBison();
}

}