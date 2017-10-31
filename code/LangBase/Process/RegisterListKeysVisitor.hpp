#include "../DescrVisitor.hpp"
#include "../LangData.hpp"
#include <queue>

namespace LangBase {

using std::queue;

/**
 * Lists are dependent on other keys for their
 * type definition, so here we have the other
 * types and are able to set list keys.
 * Lists of lists todo
 */
class RegisterListKeysVisitor : public DescrVisitor {
public:
    LData *langData;
    queue<ListNode*> unresolved;
    RegisterListKeysVisitor(LData *langData) : langData(langData) {}

    // Runs visitList until there are no more
    // lists in queue, or loop is detected.
    void visitSource(SourceNode *node) {
        DescrVisitor::visitSource(node);
        if (unresolved.size() > 0) {
            size_t prevSize = unresolved.size();
            size_t numTimesEqual = 0;
            while (unresolved.size() > 0) {
                visitList(unresolved.front());
                unresolved.pop();
                if (unresolved.size() == prevSize) {
                    ++numTimesEqual;
                    // Allow equal the same number of times
                    // as there are members in the queue.
                    if (numTimesEqual >= unresolved.size()) {
                        printf("Loop detected in lists\n");
                        while (!unresolved.empty()) {
                            printf("List: %s\n", unresolved.front()->typeDecl->identifier.c_str());
                            unresolved.pop();
                        }
                        exit(1);
                    }
                } else {
                    numTimesEqual = 0;
                }
            }
        }
    }
    void visitList(ListNode *node) {
        string grammarKey = langData->keyFromTypeDecl(node->typeDecl);
        ListGrammarType *grammarType = langData->listGrammarTypes[grammarKey];
        if (node->nodes->size() == 0) {
            TypedPart *typed1 = langData->getTypedPart(node->astKey);
            TypedPart *typed2 = langData->getTypedPart(node->tokenSep);
            // Separator between or at end
            TypedPart *listType;
            TypedPart *sepToken;
            bool sepBetween;
            // Check for a token
            // All tokens should be registered so
            // we should be able to detect a token.
            if (typed1->type == PTOKEN) {
                sepBetween = true;
                sepToken = typed1;
                listType = typed2;
            } else if (typed2->type == PTOKEN) {
                sepBetween = false;
                sepToken = typed2;
                listType = typed1;
            } else {
                printf("List requires a token and a type reference");
                printf("Token before means separator between,\n");
                printf("Token after means separator after each.\n");
                exit(1);
            }
            if (listType == nullptr) {
                // List seems to be needing another list
                // Add to queue that should be
                // passed here again
                unresolved.push(node);
                return;
            }
            grammarType->type = listType;
            grammarType->sep = sepToken;
            grammarType->sepBetween = sepBetween;
        } else {
            // List with specified rules
            // Both identifier and type is required
            if (node->typeDecl->identifier == "" || node->typeDecl->alias == "") {
                printf("List with rules requires both identifier and type\n");
                exit(1);
            }
            TypedPart *typed = langData->getTypedPart(node->typeDecl->identifier);
            if (typed == nullptr) {
                // This should be an ast class
                // but defined with this node
                // and not it's own ast node
                // Perhaps this should be added as
                // dedicated ast grammar, or
                // a dedicated list ast part
                typed = new TypedPartAst(node->typeDecl->identifier, node->typeDecl->identifier);
            }
            grammarType->type = typed;
        }
    }
};
}