#pragma once
#include <vector>
#include <string>
#include <stdarg.h>
#include <iostream>

enum NodeType {
    TOKEN_NODE,
    ENUM_NODE,
    ENUM_DECL_NODE,
    TYPE_DECL_NODE,
    AST_NODE,
    AST_DEF_NODE,
    AST_PART_NODE,
    LIST_NODE,
    SOURCE_NODE
};

enum TokenType {
    NONE,
    TSTRING,
    TINT,
    TFLOAT
};

class DescrNode {
public:
    NodeType nodeType;
    DescrNode(NodeType nodeType) : nodeType(nodeType) {
        switch (nodeType) {
            case TOKEN_NODE:
            fprintf(stdout, "token ");
            break;
            case ENUM_NODE:
            fprintf(stdout, "enum ");
            break;
            case ENUM_DECL_NODE:
            fprintf(stdout, "enum_decl ");
            break;
            case AST_NODE:
            fprintf(stdout, "ast ");
            break;
            case AST_DEF_NODE:
            fprintf(stdout, "ast_def ");
            break;
            case AST_PART_NODE:
            fprintf(stdout, "ast_part ");
            break;
            case LIST_NODE:
            fprintf(stdout, "list ");
            break;
            case SOURCE_NODE:
            fprintf(stdout, "source ");
            break;
        }
    }
};

class SourceNode : public DescrNode {
public:
    std::vector<DescrNode*> *nodes;
    SourceNode(std::vector<DescrNode*> *nodes) : DescrNode(SOURCE_NODE), nodes(nodes) {}
};

class TokenNode : public DescrNode {
public:
    TokenType type;
    std::string identifier;
    std::string regex;
    TokenNode(std::string identifier, std::string regex) 
        : DescrNode(TOKEN_NODE), type(NONE), identifier(identifier), regex(regex) {}
    TokenNode(TokenType type, std::string identifier, std::string regex) 
        : DescrNode(TOKEN_NODE), type(type), identifier(identifier), regex(regex) {}
};

class TypeDecl : public DescrNode {
public:
    std::string identifier;
    std::string alias;
    TypeDecl(std::string identifier) : DescrNode(TYPE_DECL_NODE), identifier(identifier), alias("") {}
    TypeDecl(std::string identifier, std::string alias) 
        : DescrNode(TYPE_DECL_NODE), identifier(identifier), alias(alias) {}
};

class EnumDeclNode : public DescrNode {
public:
    std::string identifier;
    std::string regex;
    EnumDeclNode(std::string identifier, std::string regex) 
        : DescrNode(ENUM_DECL_NODE), identifier(identifier), regex(regex) {}
};

class EnumNode : public DescrNode {
public:
    TypeDecl *typeDecl;
    std::vector<EnumDeclNode*> *nodes;
    EnumNode(TypeDecl *typeDecl, std::vector<EnumDeclNode*> *nodes) 
        : DescrNode(ENUM_NODE), typeDecl(typeDecl), nodes(nodes) {}
};

class AstPart : public DescrNode {
public:
    std::string identifier;
    std::string alias;
    AstPart(std::string identifier) : DescrNode(AST_PART_NODE), identifier(identifier) {}
    AstPart(std::string identifier, std::string alias) 
        : DescrNode(AST_PART_NODE), identifier(identifier), alias(alias) {}
};

class AstDef : public DescrNode {
public:
    std::string identifier;
    std::vector<AstPart*> *nodes;
    AstDef(std::string identifier, std::vector<AstPart*> *nodes) 
        : DescrNode(AST_DEF_NODE), identifier(identifier), nodes(nodes) {}
};

class AstNode : public DescrNode {
public:
    TypeDecl *typeDecl;
    std::vector<AstDef*>* nodes;
    AstNode(TypeDecl *typeDecl, std::vector<AstDef*> *nodes) 
        : DescrNode(AST_NODE), typeDecl(typeDecl), nodes(nodes) {}
};

class ListNode : public DescrNode {
public:
    std::string identifier;
    std::string astKey;
    std::string tokenSep;
    ListNode(std::string identifier, std::string astKey, std::string tokenSep) 
        : DescrNode(LIST_NODE), identifier(identifier), astKey(astKey), tokenSep(tokenSep) {}
};
