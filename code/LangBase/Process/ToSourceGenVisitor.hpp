#include <string>
#include "../LangData.hpp"
#include "../DescrNode.hpp"
#include "../DescrVisitor.hpp"
#include <map>

namespace LangBase {

using std::string;
using std::map;

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
 * Generates toSource visitor for language
 */
class ToSourceGenVisitor : public DescrVisitor {
public:
    LData *langData;
    map<string, map<string, ToSourceVisitor*>> visitors;
    ToSourceGenVisitor(LData *langData)
        : langData(langData) {}
    /**
     * Visitor cases are keyed on astClass,
     * then serialized tokens.
     * Returns a nullptr to signal
     * the combination is already present.
     */
    ToSourceVisitor* getToSourceVisitor(string astClass, string serialized) {
        if (visitors.count(astClass) == 0) {
            visitors.emplace(astClass, map<string, ToSourceVisitor*>());
        }
        if (visitors[astClass].count(serialized) > 0) {
            // Already present
            // It is assumed it would give the same result
            // with the same serialized token list.
            // Considered "already processed"
            return nullptr;
        }
        ToSourceVisitor *newVisitor = new ToSourceVisitor(astClass, langData);
        visitors[astClass].emplace(serialized, newVisitor);
        return newVisitor;
    }
    void genToSource(string baseClass, string identifier, vector<AstPart*>* parts) {
        string defClass = baseClass;
        // Identifier can refer to other ast node
        if (identifier != "") {
            TypedPart *idPart = langData->getTypedPart(identifier);
            if (idPart != nullptr && idPart->type == PAST) {
                // Let other ast visit handle this
                return;
            } else {
                defClass = identifier;
            }
        }
        ToSourceVisitor *visitor = getToSourceVisitor(defClass, langData->serializeParts(parts));
        for (AstPart *defPart : *parts) {
            TypedPart *part = langData->getTypedPart(defPart->identifier);
            part->addToVisitor(visitor);
        }
    }
    void generateVisitors() {
        // Generate visitors for ast types
        for (auto const &astType : langData->astGrammarTypes) {
            AstGrammarType *grammar = astType.second;
            string astClass = "";
            for (GrammarRule *rule : grammar->rules) {
                ToSourceVisitor *visitor = getToSourceVisitor(astClass, rule->serialized);
                for (string token : rule->tokenList) {
                    TypedPart *part = langData->getTypedPart(token);
                    part->addToVisitor(visitor);
                }
            }
        }
        // Generate visitors for list types
        for (auto const &listType : langData->listGrammarTypes) {
            ListGrammarType *grammar = listType.second;
            string astClass = "";
            for (GrammarRule *rule : grammar->rules) {
                ToSourceVisitor *visitor = getToSourceVisitor(astClass, rule->serialized);
                for (string token : rule->tokenList) {
                    TypedPart *part = langData->getTypedPart(token);
                    part->addToVisitor(visitor);
                }
            }
        }
    }
    void visitAst(AstNode *node) {
        string baseClass = node->typeDecl->identifier;
        for (AstDef *def : *node->nodes) {
            genToSource(baseClass, def->identifier, def->nodes);
        }
    }
    void visitList(ListNode *node) {
        // Skip shorthand definition
        if (node->nodes->size() == 0) {
            return;
        }
        string baseClass = node->typeDecl->identifier;
        for (ListDef *def : *node->nodes) {
            genToSource(baseClass, def->identifier, def->nodes);
        }
    }
};
}