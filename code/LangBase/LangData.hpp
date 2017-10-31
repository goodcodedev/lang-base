#pragma once
#include <string>
#include <map>
#include <set>
#include <vector>
#include "DescrNode.hpp"
#include "GrammarRule.hpp"
#include "Ast.hpp"

namespace LangBase {

using std::string;
using std::map;
using std::set;
using std::vector;

class AstGrammarType;
class ListGrammarType;
class EnumGrammarType;
class StartAction;

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
    // First keyed on astClass, then a map with
    // serialized token list, with the token list.
    map<string, map<string, vector<string>>> serializedTokenLists;
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
    string serializeTokenList(vector<string> tokenList) {
        // Simple serialization (readable, could also check for uniqueness)
        // Consider base64 or something
        string serialized = "";
        bool isFirst = true;
        for (string token : tokenList) {
            if (!isFirst) serialized += "_";
            serialized += token;
        }
        serialized += "_S";
        return serialized;
    }
    string serializeParts(vector<AstPart*> *parts) {
        vector<string> tokenList;
        for (AstPart *part : *parts) {
            tokenList.push_back(part->identifier);
        }
        return serializeTokenList(tokenList);
    }
    string addSerializedTokenList(string astClass, vector<string> tokenList) {
        if (serializedTokenLists.count(astClass) == 0) {
            serializedTokenLists.emplace(astClass, map<string, vector<string>>());
        }
        string serialized = serializeTokenList(tokenList);
        if (serializedTokenLists[astClass].count(serialized) == 0) {
            serializedTokenLists[astClass].emplace(serialized, tokenList);
        }
        return serialized;
    }
};
}