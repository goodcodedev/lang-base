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
class ToSourceCase {
public:
    LData *langData;
    string code;
    string serialized;
    string grammarKey;
    bool isClassKey;
    ToSourceCase(LData *langData, string grammarKey, bool isClassKey)
        : langData(langData), grammarKey(grammarKey), isClassKey(isClassKey) {}
};

/**
 * Generates toSource visitor for language
 */
class ToSourceGenVisitor : public DescrVisitor {
public:
    LData *langData;
    // These will contain most code to output
    // source, additionally lists will need
    // special logic.
    map<string, vector<ToSourceCase*>> keys;
    ToSourceGenVisitor(LData *langData)
        : langData(langData) {}

    void addToKey(string key, ToSourceCase *c) {
        if (keys.count(key) == 0) {
            keys.emplace(key, vector<ToSourceCase*>());
        }
        keys[key].push_back(c);
    }
    void visitAst(AstNode *node) {
        string grammarKey = langData->keyFromTypeDecl(node->typeDecl);
        bool isClassKey = (node->typeDecl->alias == ""
                            || node->typeDecl->alias == node->typeDecl->identifier);
        AstGrammarType *grammar = langData->astGrammarTypes[grammarKey];
        for (AstRuleDef *ruleDef : grammar->ruleDefs) {
            // Generate code for each ruleDef
            ToSourceCase *c = new ToSourceCase(langData, grammarKey, isClassKey);
            for (TypedPart *part : ruleDef->typedPartList) {
                part->addToVisitor(c);
            }
            if (isClassKey) {
                addToKey(grammarKey, c);
            } else {
                addToKey(grammarKey, c);
            }
        }
    }
    void visitList(ListNode *node) {
        string grammarKey = langData->keyFromTypeDecl(node->typeDecl);
        ListGrammarType *grammar = langData->listGrammarTypes[grammarKey];
        for (ListRuleDef *ruleDef : grammar->ruleDefs) {
            ToSourceCase *c = new ToSourceCase(langData, grammarKey, false);
            if (ruleDef->astRule != nullptr) {
                for (TypedPart *part : ruleDef->astRule->typedPartList) {
                    part->addToVisitor(c);
                }
            }
            addToKey(grammarKey, c);
        }
    }
};
}