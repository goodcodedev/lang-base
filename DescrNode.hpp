#pragma once
#include <vector>
#include <string>
#include <stdarg.h>
#include <iostream>

enum NodeType {
    TOKEN_NODE,
    ENUM_NODE,
    ENUM_DECL_NODE,
    AST_NODE,
    AST_DEF_NODE,
    AST_PART_NODE,
    LIST_NODE,
    SOURCE_NODE
};

class DescrNode {
public:
    NodeType nodeType;
    DescrNode(NodeType nodeType) : nodeType(nodeType) {}
};

class SourceNode : public DescrNode {
public:
    std::vector<DescrNode*> *nodes;
    SourceNode(std::vector<DescrNode*> *nodes) : DescrNode(SOURCE_NODE), nodes(nodes) {}
};

class TokenNode : public DescrNode {
public:
    std::string identifier;
    std::string regex;
    TokenNode(std::string identifier, std::string regex) 
        : DescrNode(TOKEN_NODE), identifier(identifier), regex(regex) {}
};

class EnumNode : public DescrNode {
public:
    std::vector<EnumDeclNode*> *nodes;
    EnumNode(std::vector<EnumDeclNode*> *nodes) 
        : DescrNode(ENUM_NODE), nodes(nodes) {}
};

class EnumDeclNode : public DescrNode {
public:
    std::string identifier;
    std::string regex;
    EnumDeclNode(std::string identifier, std::string regex) 
        : DescrNode(ENUM_DECL_NODE), identifier(identifier), regex(regex) {}
};

class AstNode : public DescrNode {
public:
    std::string alias;
    std::string identifier;
    std::vector<AstDef*> *nodes;
    AstNode(std::string identifier, std::vector<AstDef*> *nodes) 
        : DescrNode(AST_NODE), alias(""), identifier(identifier), nodes(nodes) {}
    AstNode(std::string alias, std::string identifier, std::vector<AstDef*> *nodes) 
        : DescrNode(AST_NODE), alias(alias), identifier(identifier), nodes(nodes) {}
};

class AstDef : public DescrNode {
public:
    std::string identifier;
    std::vector<AstPart*> *nodes;
    AstNode(std::string identifier, std::vector<AstPart*> *nodes) 
        : DescrNode(AST_DEF_NODE), identifier(identifier), nodes(nodes) {}
};

class AstPart : public DescrNode {
public:
    std::string identifier;
    AstPart(std::string identifier) : DescrNode(AST_PART_NODE), identifier(identifier) {}
};

class ListNode : public DescrNode {
public:
    std::string identifier;
    std::string astKey;
    std::string tokenSep;
    ListNode(std::string identifier, std::string astKey, std::string tokenSep) 
        : DescrNode(LIST_NODE), identifier(identifier), astKey(astKey), tokenSep(tokenSep) {}
};

class DescrVisitor {
public:
    void visitSource(SourceNode *node) {
        for (DescrNode *node : *nodes) {
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
            }
        }
    }
    void visitToken(TokenNode *node) {
    }
    void visitEnum(EnumNode *node) {

    }
    void visitAst(AstNode *ast) {

    }
    void visitList(ListNode *node) {

    }

};