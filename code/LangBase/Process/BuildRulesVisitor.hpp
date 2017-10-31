#include "../DescrVisitor.hpp"
#include "../LangData.hpp"

namespace LangBase {

// Builds rules
// Also sets up start node
class BuildRulesVisitor : public DescrVisitor {
public:
    LData *langData;
    BuildRulesVisitor(LData *langData) : langData(langData) {}
    void visitStart(StartNode *node) {
        langData->startKey = node->identifier;
        langData->startAction = new StartAction(langData->getTypedPart(langData->startKey));
    }
    void visitEnum(EnumNode *node) {
        // Build rules
        string grammarKey = langData->keyFromTypeDecl(node->typeDecl);
        EnumGrammarType *grammarType = langData->ensureEnumGrammar(grammarKey);
        // Enums are one of several token values
        for (EnumDeclNode *enumDecl : *node->nodes) {
            GrammarRule *rule = new GrammarRule();
            // Use typed part to get possibly
            // changed identifier
            rule->tokenList = vector<string> { enumDecl->identifier };
            rule->action = new EnumValueAction(enumDecl->identifier);
            grammarType->rules.push_back(rule);
        }
    }

    GrammarRule* generateRule(string baseAstClass, string identifier, vector<AstPart*> *parts, string sepBefore, string sepAfter) {
        string defClass = baseAstClass;
        // Defs may have an identifier, this would refer
        // to a subclass of the baseClass or another ast
        // rule. (only next rule is checked, but top level
        // rules should have the type specified)
        // When rules refer to args/parts
        // that is expected in parens.
        bool isKeyRef = false;
        if (identifier.compare("") != 0) {
            // Check for other ast rule
            if (langData->astGrammarTypes.count(identifier) != 0) {
                // Get class name from other type
                defClass = langData->astGrammarTypes[identifier]->astClass;
                isKeyRef = true;
            } else {
                // Else use identifier as class
                defClass = identifier;
            }
        }
        GrammarRule *curRule = new GrammarRule();
        if (isKeyRef) {
            // Rule just need to use referred rule as part
            // which should return ast object
            curRule->tokenList.push_back(identifier);
            curRule->action = new RefAction(1, new TypedPartAst(
                identifier,
                defClass
            ));
        } else {
            // Collect rule args
            vector<RuleArg> ruleArgs;
            int num = 0;
            vector<string> tokenList;
            if (sepBefore != "") {
                // Add actual token after we get
                // serialized tokens. Separators
                // are handled besides to allow
                // moving ast parts to/from places
                // where they are not combined
                // with separators.
                ++num;
            }
            for (AstPart *part : *parts) {
                TypedPart *typedPart = langData->getTypedPart(part->identifier);
                // Skip WS (whitespace) token as this is ignored in grammar
                if (typedPart->type == PTOKEN && typedPart->identifier == "WS") continue;
                ++num;
                // Just setting alias here
                // Used as key to ast member and constructor args
                typedPart->alias = (part->alias != "") ? part->alias : part->identifier;
                if (typedPart == nullptr) {
                    printf("Key not found: %s\n", part->identifier.c_str());
                    exit(1);
                }
                tokenList.push_back(typedPart->identifier);
                if (typedPart->type == PTOKEN || typedPart == nullptr) {
                    // Skip const literal tokens
                    continue;
                }
                // Add rule arg
                ruleArgs.push_back(RuleArg(num, typedPart));
            }
            curRule->serialized = langData->addSerializedTokenList(defClass, tokenList);
            if (sepBefore != "") {
                TypedPart *typedPart = langData->getTypedPart(sepBefore);
                if (typedPart->type != PTOKEN) {
                    printf("Separator must be token");
                    exit(1);
                }
                tokenList.insert(tokenList.begin(), typedPart->identifier);
                // Num alreade incremented
            }
            if (sepAfter != "") {
                TypedPart *typedPart = langData->getTypedPart(sepAfter);
                if (typedPart->type != PTOKEN) {
                    printf("Separator must be token");
                    exit(1);
                }
                tokenList.push_back(typedPart->identifier);
                ++num;
            }
            curRule->tokenList = tokenList;
            printf("Adding constr action: %s\n", defClass.c_str());
            curRule->action = new AstConstructionAction(
                defClass,
                ruleArgs,
                curRule->serialized
            );
        }
        return curRule;
    }

    void visitAst(AstNode *node) {
        string grammarKey = langData->keyFromTypeDecl(node->typeDecl);
        // Get current grammar
        AstGrammarType *grammar = langData->ensureAstGrammar(grammarKey);
        // Get ast base class
        string baseAstClass = node->typeDecl->identifier;
        // Go through refs and collect rules
        for (AstDef* astDef : *node->nodes) {
            grammar->rules.push_back(generateRule(
                baseAstClass,
                astDef->identifier,
                astDef->nodes,
                "", ""
            ));
        }
    }
    void visitList(ListNode *node) {
        string grammarKey = langData->keyFromTypeDecl(node->typeDecl);
        ListGrammarType *grammar = langData->ensureListGrammar(grammarKey);
        if (node->nodes->size() > 0) {
            string baseAstClass = node->typeDecl->identifier;
            for (ListDef* listDef : *node->nodes) {
                grammar->rules.push_back(generateRule(
                    baseAstClass,
                    listDef->identifier,
                    listDef->nodes,
                    listDef->sepBefore,
                    listDef->sepAfter
                ));
            }
        } else {
            TypedPart *listType = grammar->type;
            TypedPart *sepToken = grammar->sep;
            bool sepBetween = grammar->sepBetween;
            // Init list
            GrammarRule *initRule = new GrammarRule();
            initRule->action = new ListInitAction(listType);
            grammar->rules.push_back(initRule);
            if (sepBetween) {
                GrammarRule *firstPart = new GrammarRule();
                firstPart->tokenList = vector<string>{
                    node->typeDecl->identifier,
                    listType->identifier
                };
                firstPart->action = new ListPushAction(1, 2, listType);
                grammar->rules.push_back(firstPart);
                GrammarRule *sepPart = new GrammarRule();
                sepPart->tokenList = vector<string>{
                    node->typeDecl->identifier,
                    sepToken->identifier,
                    listType->identifier
                };
                sepPart->action = new ListPushAction(1, 3, listType);
                grammar->rules.push_back(sepPart);
            } else {
                GrammarRule *sepPart = new GrammarRule();
                sepPart->tokenList = vector<string>{
                    node->typeDecl->identifier,
                    listType->identifier,
                    sepToken->identifier
                };
                sepPart->action = new ListPushAction(1, 2, listType);
                grammar->rules.push_back(sepPart);
            }
        }
    }
};
}