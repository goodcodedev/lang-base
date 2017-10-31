#include "../DescrVisitor.hpp"
#include "../LangData.hpp"

namespace LangBase {

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
        string grammarKey = langData->keyFromTypeDecl(node->typeDecl);
        langData->ensureListGrammar(grammarKey);
    }
};
}