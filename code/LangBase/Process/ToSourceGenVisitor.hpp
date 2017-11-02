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
    map<string, vector<ToSourceCase*>> classCases;
    // List cases
    // Mapped by list key, then 
    // astClass mapped to list of
    // serialized tokenLists
    // useful to detect when it is needed to
    // branch based on tokenlist
    // It could be useful/nice
    // to have cases ordered like in
    // the source besides this.
    // Class cases in ast type
    // It could be only a subset of
    // classes allowed in a type,
    // restricting to the same set
    // as parsing makes sense.
    map<string, map<string, vector<string>>> keyedCases;
    ToSourceGenVisitor(LData *langData)
        : langData(langData) {}

    // Add class ToSourceCase
    void addClassCase(string astClass, ToSourceCase *c) {
        if (classCases.count(astClass) == 0) {
            classCases.emplace(astClass, vector<ToSourceCase*>());
        }
        classCases[astClass].push_back(c);
    }
    // Add keyed (list or ast), astClass and
    // serialized tokenList
    void addKeyedItem(string key, string astClass, string serialized) {
        if (keyedCases.count(key) == 0) {
            keyedCases.emplace(key, map<string, vector<string>>());
        }
        if (keyedCases[key].count(astClass) == 0) {
            keyedCases[key].emplace(astClass, vector<string>());
        }
        keyedCases[key][astClass].push_back(serialized);
    }
    void visitAst(AstNode *node) {
        string grammarKey = langData->keyFromTypeDecl(node->typeDecl);
        bool isClassKey = (node->typeDecl->alias == ""
                            || node->typeDecl->alias == node->typeDecl->identifier);
        AstGrammarType *grammar = langData->astGrammarTypes[grammarKey];
        for (AstRuleDef *ruleDef : grammar->ruleDefs) {
            // If this is a reference, continue
            if (ruleDef->refType != nullptr) continue;
            // Generate code for each ruleDef
            ToSourceCase *c = new ToSourceCase(langData, grammarKey, isClassKey);
            for (TypedPart *part : ruleDef->typedPartList) {
                part->addToVisitor(c);
            }
            addClassCase(ruleDef->astClass, c);
            addKeyedItem(grammarKey, ruleDef->astClass, ruleDef->serialized);
        }
    }
    void visitList(ListNode *node) {
        string grammarKey = langData->keyFromTypeDecl(node->typeDecl);
        ListGrammarType *grammar = langData->listGrammarTypes[grammarKey];
        string baseClass = node->typeDecl->identifier;
        for (ListRuleDef *ruleDef : grammar->ruleDefs) {
            // If this is a reference, continue
            if (ruleDef->astRule != nullptr && ruleDef->astRule->refType != nullptr) continue;
            string astClass = baseClass;
            string serialized = "";
            ToSourceCase *c = new ToSourceCase(langData, grammarKey, false);
            if (ruleDef->astRule != nullptr) {
                astClass = ruleDef->astRule->astClass;
                serialized = ruleDef->astRule->serialized;
                for (TypedPart *part : ruleDef->astRule->typedPartList) {
                    part->addToVisitor(c);
                }
            }
            addClassCase(astClass, c);
            addKeyedItem(grammarKey, astClass, serialized);
        }
    }
};
}