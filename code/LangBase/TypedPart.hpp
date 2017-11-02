#pragma once
#include <string>

namespace LangBase {

using std::string;

class ToSourceCase;
class LData;

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
    virtual void addToVisitor(ToSourceCase *visitor) = 0;
};

// Token part
class TypedPartToken : public TypedPart {
public:
    TypedPartToken(string identifier)
        : TypedPart(PTOKEN, identifier) {}
    void generateGrammarVal(string *str, int num, LData *langData);
    void generateGrammarType(string *str, LData *langData);
    void addToVisitor(ToSourceCase *visitor);
    string getCleanedVal(LData *langData);
};
// Prim token part
class TypedPartPrim : public TypedPart {
public:
    TypedPartPrim(PartType type, string identifier)
        : TypedPart(type, identifier) {}
    void generateGrammarVal(string *str, int num, LData *langData);
    void generateGrammarType(string *str, LData *langData);
    void addToVisitor(ToSourceCase *visitor);
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
    void addToVisitor(ToSourceCase *visitor);
};
// Ast part
class TypedPartAst : public TypedPart {
public:
    string astClass;
    TypedPartAst(string identifier, string astClass) 
        : TypedPart(PAST, identifier), astClass(astClass) {}
    void generateGrammarVal(string *str, int num, LData *langData);
    void generateGrammarType(string *str, LData *langData);
    void addToVisitor(ToSourceCase *visitor);
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
    void addToVisitor(ToSourceCase *visitor);
};
}