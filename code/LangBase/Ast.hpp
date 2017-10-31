#pragma once
#include <string>
#include <set>
#include <map>
#include <vector>
#include "TypedPart.hpp"

namespace LangBase {

using std::string;
using std::map;
using std::set;
using std::vector;

class LData;

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
    string serialized;
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
}