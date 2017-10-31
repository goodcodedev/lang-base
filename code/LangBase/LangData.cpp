#include "LangData.hpp"
#include "Ast.hpp"
#include "TypedPart.hpp"

namespace LangBase {

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
}