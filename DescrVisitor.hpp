#pragma once
#include "DescrNode.hpp"

class DescrVisitor {
public:
    virtual void visitSource(SourceNode *node) {
        for (DescrNode *node : *node->nodes) {
            switch (node->nodeType) {
                case TOKEN_NODE:
                visitToken(static_cast<TokenNode*>(node));
                break;
                case ENUM_NODE:
                visitEnum(static_cast<EnumNode*>(node));
                break;
                case AST_NODE:
                visitAst(static_cast<AstNode*>(node));
                break;
                case LIST_NODE:
                visitList(static_cast<ListNode*>(node));
                break;
                case START_NODE:
                visitStart(static_cast<StartNode*>(node));
                break;
                default: {
                    printf("Unrecognized source node");
                    exit(1);
                }
            }
        }
    }
    virtual void visitStart(StartNode *node) {

    }
    virtual void visitTypeDecl(TypeDecl *node) {

    }
    virtual void visitToken(TokenNode *node) {
    }
    virtual void visitEnumDecl(EnumDeclNode *node) {

    }
    virtual void visitEnum(EnumNode *node) {
        visitTypeDecl(node->typeDecl);
        for (EnumDeclNode *enumDecl : *node->nodes) {
            visitEnumDecl(enumDecl);
        }
    }
    virtual void visitAstPart(AstPart *node) {

    }
    virtual void visitAstDef(AstDef *node) {
        for (AstPart *astPart : *node->nodes) {
            visitAstPart(astPart);
        }
    }
    virtual void visitAst(AstNode *node) {
        visitTypeDecl(node->typeDecl);
        for (AstDef *astDef : *node->nodes) {
            visitAstDef(astDef);
        }
    }
    virtual void visitList(ListNode *node) {

    }

};