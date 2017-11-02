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

    /**
     * Adds tokens and wraps types action in list
     * actions
     * There needs to be several rules for lists
     * When the separator is between elements,
     * one rule is: listKey element,
     * and another: listKey SEPARATOR element/ast tokens
     * This method creates a particular rule,
     * and will be passed withSep=false to exclude
     * the separator
     */
    RuleAction* generateListRule(ListGrammarType *grammar, ListRuleDef *listDef, GrammarRule *rule, bool withSep) {
        rule->tokenList.push_back(grammar->key);
        if (withSep && listDef->sepBetween != nullptr) {
            rule->tokenList.push_back(listDef->sepBetween->identifier);
        }
        if (listDef->astRule == nullptr) {
            printf("List requires ast rule currently");
            exit(1);
        }
        RuleAction *innerAction = generateAstRule(rule, listDef->astRule);
        if (listDef->sepAfter != nullptr) {
            rule->tokenList.push_back(listDef->sepAfter->identifier);
        }
        TypedPart *pushType;
        switch (innerAction->type) {
            case RARef: {
                RefAction *refAction = static_cast<RefAction*>(innerAction);
                pushType = refAction->ref;
            }
            break;
            case RAAstConstruction: {
                AstConstructionAction *astAction = static_cast<AstConstructionAction*>(innerAction);
                pushType = astAction->typed;
            }
            break;
            default: {
                printf("Could not resolve type\n");
                exit(1);
            }
            break;
        }
        ListPushAction *pushAction = new ListPushAction(1, innerAction, pushType, grammar->type);
        pushAction->innerAction = innerAction;
        return pushAction;
    }

    /**
     * Create grammar rule based on ast and optionally list rule def
     * Should probably be more composable to allow lists of lists
     */
    RuleAction* generateAstRule(GrammarRule *rule, AstRuleDef *ruleDef) {
        // If this is part of a list, tokens
        // might be added.
        int num =  rule->tokenList.size();
        int startNum = num;
        if (ruleDef->refType != nullptr) {
            // Rule just need to use referred rule as part
            // which should return ast object
            rule->tokenList.push_back(ruleDef->refType->identifier);
            ++num;
            RefAction *refAction = new RefAction(num, new TypedPartAst(
                ruleDef->refType->identifier,
                ruleDef->astClass
            ));
            return refAction;
        }
        // Collect rule args
        vector<RuleArg> ruleArgs;
        for (TypedPart *typedPart : ruleDef->typedPartList) {
            // Skip whitespace token
            // We use it to signal significant whitespace
            // and don't need to add it to grammar token list
            if (typedPart->type == PTOKEN && typedPart->identifier == "WS") continue;
            rule->tokenList.push_back(typedPart->identifier);
            ++num;
            if (typedPart->type == PTOKEN) {
                // Don't add args from literal token (there could be exceptions
                // where it would be nice to store it. Maybe use alias to signal this)
                continue;
            }
            // Add rule arg
            ruleArgs.push_back(RuleArg(num, typedPart));
        }
        // Serialized without separators
        rule->serialized = langData->addSerializedTokenList(
            ruleDef->astClass, 
            vector<string>(rule->tokenList.begin() + startNum, rule->tokenList.end())
        );
        AstConstructionAction *astAction = new AstConstructionAction(
            ruleDef->astClass,
            ruleArgs,
            rule->serialized
        );
        astAction->typed = new TypedPartAst(ruleDef->astClass, ruleDef->astClass);
        return astAction;
    }

    void visitAst(AstNode *node) {
        string grammarKey = langData->keyFromTypeDecl(node->typeDecl);
        // Get current grammar
        AstGrammarType *grammar = langData->ensureAstGrammar(grammarKey);
        // Go through defs and collect rules
        for (AstRuleDef* ruleDef : grammar->ruleDefs) {
            GrammarRule *rule = new GrammarRule();
            rule->action = generateAstRule(rule, ruleDef);
            grammar->rules.push_back(rule);
        }
    }
    void visitList(ListNode *node) {
        string grammarKey = langData->keyFromTypeDecl(node->typeDecl);
        ListGrammarType *grammar = langData->ensureListGrammar(grammarKey);
        // Init list
        // This may not be the best way
        // as it can give ambiguous grammars
        // Trick for now
        // Rather initialize list in rule matching
        // a first element. However, not sure then
        // how to capture empty list.
        GrammarRule *initRule = new GrammarRule();
        initRule->action = new ListInitAction(grammar->type);
        grammar->rules.push_back(initRule);
        if (grammar->sepBetween) {
            for (ListRuleDef *ruleDef : grammar->ruleDefs) {
                // With current scheme, we need one
                // rule without separator, and one
                // with separator.
                GrammarRule *rule = new GrammarRule();
                rule->action = generateListRule(grammar, ruleDef, rule, false);
                grammar->rules.push_back(rule);
                GrammarRule *rule2 = new GrammarRule();
                rule2->action = generateListRule(grammar, ruleDef, rule2, true);
                grammar->rules.push_back(rule2);
            }
        } else {
            // Sep after
            // Rules need list accumulation and
            // optionally specified sep after
            for (ListRuleDef *ruleDef : grammar->ruleDefs) {
                // With current scheme, we need one
                // rule without separator, and one
                // with separator.
                GrammarRule *rule = new GrammarRule();
                rule->action = generateListRule(grammar, ruleDef, rule, true);
                grammar->rules.push_back(rule);
            }
        }
    }
};
}