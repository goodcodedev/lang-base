#include "../DescrVisitor.hpp"
#include "../LangData.hpp"
#include "../Ast.hpp"

namespace LangBase {

class BuildAstVisitor : public DescrVisitor {
public:
    LData *langData;
    BuildAstVisitor(LData *langData) : langData(langData) {}
    void visitEnum(EnumNode *node) {
        AstEnum* astEnum = langData->ensureEnum(node->typeDecl->identifier);
        for (EnumDeclNode *enumDecl : *node->nodes) {
            astEnum->members.insert(enumDecl->identifier);
            astEnum->values.emplace(enumDecl->identifier, enumDecl->regex);
        }
    }

    void buildFromRule(GrammarRule *rule, string baseAstName) {
        // If rule is a ref, register referenced
        // ast class as subclass.
        // Members and constructors are set up when
        // visiting referenced ast node.
        if (rule->action->type == RARef) {
            RefAction *refAction = static_cast<RefAction*>(rule->action);
            if (refAction->ref->type != PAST) {
                printf("Can only handle ref to ast");
                exit(1);
            }
            // Ensure subclass relationship
            TypedPartAst *refType = static_cast<TypedPartAst*>(refAction->ref);
            langData->ensureSubRelation(baseAstName, refType->astClass);
            return;
        } else if (rule->action->type != RAAstConstruction) {
            printf("Can only handle ref and ast construction actions");
            exit(1);
        }
        AstConstructionAction *action = static_cast<AstConstructionAction*>(rule->action);
        AstClass *ruleClass = langData->ensureSubRelation(baseAstName, action->astClass);
        // Ensure class has members for all args
        for (RuleArg ruleArg : action->args) {
            TypedPart *typedPart = ruleArg.typedPart;
            string memberKey = typedPart->getMemberKey();
            if (ruleClass->members.count(memberKey) != 0) {
                // This equality test will check identifier, alias and type
                // It should probably test type more thoroughly, and
                // rather check resolved key todo
                if (*ruleClass->members[memberKey]->typedPart != *typedPart) {
                    // Member has different type
                    printf("Member has different type");
                    exit(1);
                }
            } else {
                ruleClass->members.emplace(
                    memberKey,
                    new AstClassMember(typedPart)
                );
            }
        }
        // Ensure ast class constructor
        bool constructorFound = false;
        for (AstClassConstructor *constr : ruleClass->constructors) {
            if (constr->args.size() == action->args.size()) {
                // Do simple equality check.
                // Concievably we could try to reorder, but
                // that could also lead to more volatility
                // in available constructors
                // Perhaps when the ast interface is defined,
                // reorder could be tried.
                bool isEqual = true;
                isEqual = (constr->serialized == rule->serialized);
                /*
                for (size_t i = 0; i < constr->args.size(); ++i) {
                    // Assume correspondance with member field types
                    if (constr->args[i] != action->args[i].typedPart->getMemberKey()) {
                        isEqual = false;
                        break;
                    }
                }
                */
                if (isEqual) {
                    constructorFound = true;
                    break;
                }
            }
        }
        if (!constructorFound) {
            // Create constructor based on ruleArgs
            AstClassConstructor *constr = new AstClassConstructor();
            for (RuleArg ruleArg : action->args) {
                constr->args.push_back(ruleArg.typedPart->getMemberKey());
            }
            constr->serialized = rule->serialized;
            ruleClass->constructors.push_back(constr);
        }
    }

    void visitAst(AstNode *node) {
        string grammarKey = langData->keyFromTypeDecl(node->typeDecl);
        // Get current grammar
        AstGrammarType *grammar = langData->ensureAstGrammar(grammarKey);
        // Get ast base class
        string baseAstName = node->typeDecl->identifier;
        langData->ensureClass(baseAstName);
        // Go through rules and ensure ast classes
        // has needed members and constructors
        for (GrammarRule *rule : grammar->rules) {
            buildFromRule(rule, baseAstName);
        }
    }

    void visitList(ListNode *node) {
        if (node->astKey != "" || node->tokenSep != "") {
            return;
        }
        string grammarKey = langData->keyFromTypeDecl(node->typeDecl);
        // Get current grammar
        ListGrammarType *grammar = langData->ensureListGrammar(grammarKey);
        // Get ast base class
        string baseAstName = node->typeDecl->identifier;
        langData->ensureClass(baseAstName);
        // Go through rules and ensure ast classes
        // has needed members and constructors
        for (GrammarRule *rule : grammar->rules) {
            buildFromRule(rule, baseAstName);
        }
    }
};
}