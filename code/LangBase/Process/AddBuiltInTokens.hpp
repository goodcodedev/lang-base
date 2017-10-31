#include "../DescrVisitor.hpp"
#include "../LangData.hpp"

namespace LangBase {

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
        if (node->astKey != "") {
            TypedPart *typed1 = langData->getTypedPart(node->astKey);
            if (typed1 == nullptr) langData->addBuiltInToken(node->astKey);
        }
        if (node->tokenSep != "") {
            TypedPart *typed2 = langData->getTypedPart(node->tokenSep);
            if (typed2 == nullptr) langData->addBuiltInToken(node->tokenSep);
        }
        // Visit the parts in list
        DescrVisitor::visitList(node);
    }
    void visitListDef(ListDef *node) {
        if (node->sepBefore != "") {
            TypedPart *typed = langData->getTypedPart(node->sepBefore);
            if (typed == nullptr) langData->addBuiltInToken(node->sepBefore);
        }
        if (node->sepAfter != "") {
            TypedPart *typed = langData->getTypedPart(node->sepAfter);
            if (typed == nullptr) langData->addBuiltInToken(node->sepAfter);
        }
        DescrVisitor::visitListDef(node);
    }
};
}