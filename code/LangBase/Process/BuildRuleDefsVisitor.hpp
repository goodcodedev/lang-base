#include "../DescrVisitor.hpp"
#include "../LangData.hpp"

namespace LangBase {

/** 
 * Builds rule defs
 * These are processed from ast to describe
 * rules with metadata.
 * There is not a 1:1 correspondance with
 * grammar rules, as there may be
 * several grammar rules to implement
 * for example a list.
 * They are processed after keys are registered,
 * so they are able to register types
 * for various entities.
 * Possibly, this could be added to
 * ast classes, list, ast. Separating it
 * into dedicated classes currently.
 * They should be useful when creating
 * toString, and possibly other code.
 */
class BuildRuleDefsVisitor : public DescrVisitor {
public:
    LData *langData;
    BuildRuleDefsVisitor(LData *langData) : langData(langData) {}

    AstRuleDef* generateAstDef(string baseAstClass, string identifier, vector<AstPart*> *parts) {
        string defClass = baseAstClass;
        AstRuleDef *ruleDef = new AstRuleDef();
        ruleDef->baseClass = baseAstClass;
        // Defs may have an identifier, this would refer
        // to a subclass of the baseClass or another ast
        // rule. (only next rule is checked, but top level
        // rules should have the type specified)
        // When rules refer to args/parts
        // that is expected in parens.
        bool isKeyRef = false;
        if (identifier.compare("") != 0) {
            // Check for other ast rule
            TypedPart *typedRef = langData->getTypedPart(identifier);
            if (typedRef != nullptr) {
                if (typedRef->type == PAST) {
                    TypedPartAst *typedAst = static_cast<TypedPartAst*>(typedRef);
                    ruleDef->refType = typedAst;
                    ruleDef->astClass = typedAst->astClass;
                    ruleDef->tokenList.push_back(identifier);
                    ruleDef->typedPartList.push_back(typedAst);
                    return ruleDef;
                } else {
                    printf("Only ast supported as refs in ast rules");
                    exit(1);
                }
            } else {
                // If not a reference, use identifier as
                // class name
                defClass = identifier;
            }
        }
        // Collect tokenList and typedParts from parts
        for (AstPart *part : *parts) {
            // This may include WS token, which should not
            // be in the final grammar list
            ruleDef->tokenList.push_back(part->identifier);
            TypedPart *typedPart = langData->getTypedPart(part->identifier);
            if (typedPart == nullptr) {
                printf("Key not found: %s\n", part->identifier.c_str());
                exit(1);
            }
            // Register alias here
            typedPart->alias = (part->alias != "") ? part->alias : part->identifier;
            ruleDef->typedPartList.push_back(typedPart);
        }
        ruleDef->serialized = langData->addSerializedTokenList(ruleDef->astClass, ruleDef->tokenList);
        ruleDef->astClass = defClass;
        return ruleDef;
    }

    void visitAst(AstNode *node) {
        string grammarKey = langData->keyFromTypeDecl(node->typeDecl);
        // Get current grammar
        AstGrammarType *grammar = langData->ensureAstGrammar(grammarKey);
        // Get ast base class
        string baseAstClass = node->typeDecl->identifier;
        // Collect rules defs
        for (AstDef* astDef : *node->nodes) {
            grammar->ruleDefs.push_back(generateAstDef(baseAstClass, astDef->identifier, astDef->nodes));
        }
    }
    void visitList(ListNode *node) {
        string grammarKey = langData->keyFromTypeDecl(node->typeDecl);
        ListGrammarType *grammar = langData->ensureListGrammar(grammarKey);
        if (node->nodes->size() > 0) {
            // We have a list of defs
            string baseAstClass = node->typeDecl->identifier;
            for (ListDef* listDef : *node->nodes) {
                ListRuleDef *ruleDef = new ListRuleDef();
                // Currently only handles ast
                AstRuleDef *astRule = generateAstDef(baseAstClass, listDef->identifier, listDef->nodes);
                ruleDef->astRule = astRule;
                ruleDef->tokenList = astRule->tokenList;
                // Also only sepAfter in this case. Not sure
                // if it makes sense/how it makes sense with between
                // specified here. It could be specified on
                // the type though (todo)
                if (listDef->sepAfter != "") {
                    TypedPart *sepAfter = langData->getTypedPart(listDef->sepAfter);
                    ruleDef->sepAfter = sepAfter;
                    ruleDef->tokenList.push_back(sepAfter->identifier);
                }
                ruleDef->serialized = langData->addSerializedTokenList(astRule->astClass, ruleDef->tokenList);
                grammar->ruleDefs.push_back(ruleDef);
            }
        } else {
            TypedPart *listType = grammar->type;
            TypedPart *sepToken = grammar->sep;
            bool sepBetween = grammar->sepBetween;
            ListRuleDef *ruleDef = new ListRuleDef();
            // Currently assume ref in type
            if (listType->type != PAST) {
                printf("Currenly only ast supported");
                exit(1);
            }
            AstRuleDef *astRule = new AstRuleDef();
            astRule->refType = static_cast<TypedPartAst*>(listType);
            astRule->astClass = astRule->refType->astClass;
            astRule->tokenList.push_back(astRule->refType->identifier);
            ruleDef->astRule = astRule;
            ruleDef->tokenList = astRule->tokenList;
            if (sepBetween) {
                ruleDef->sepBetween = sepToken;
                ruleDef->tokenList.insert(ruleDef->tokenList.begin(), sepToken->identifier);
            } else {
                ruleDef->sepAfter = sepToken;
                ruleDef->tokenList.push_back(sepToken->identifier);
            }
            grammar->ruleDefs.push_back(ruleDef);
        }
    }
};
}