#include "../DescrVisitor.hpp"
#include "../DescrNode.hpp"
#include "../LangData.hpp"
#include "../Ast.hpp"
#include "RegisterKeysVisitor.hpp"
#include "RegisterListKeysVisitor.hpp"
#include "AddBuiltInTokens.hpp"
#include "BuildRuleDefsVisitor.hpp"
#include "BuildRulesVisitor.hpp"
#include "BuildAstVisitor.hpp"
#include "ToSourceGenVisitor.hpp"
#include <iostream>
#include <fstream>
#include <array>

extern FILE *yyin;
extern int yyparse();
extern LangBase::DescrNode *result; 

namespace LangBase {

class SourceGenerator {
public:
    LData *langData;
    string folder;
    SourceGenerator(LData *langData, string folder) 
        : langData(langData), folder(folder) {}

    void saveToFile(string *str, string fileName) {
        std::ofstream f;
        f.open(folder + "/" + fileName);
        f << *str;
        printf("%s", str->c_str());
        f.close();
    }
    // Generate flex file
    void generateLexFile() {
        string str = "";
        str +=  "%{\n";
        str += "#include \"" + langData->langKey + ".tab.h\"\n";
        str +=  "#define register // Deprecated in c++11\n"
                "#ifdef _WIN32\n"
                "   #define __strdup _strdup\n"
                "#else\n"
                "   #define __strdup strdup\n"
                "#endif\n"
                "%}\n"
                "%option yylineno\n"
                "%%\n";
        for (auto const &pair : langData->tokenData) {
            TokenData *token = pair.second;
            if (token->key == "WS") {
                // Used to signal whitespace needed
                // in source code
                continue;
            }
            switch (token->type) {
                case NONE:
                str += token->regex + " { return " + token->getGrammarToken() + "; }\n";
                break;
                case TINT:
                str += token->regex + " { yylval.ival = atoi(yytext); return " + token->getGrammarToken() + "; }\n";
                break;
                case TSTRING:
                str += token->regex + " { yylval.sval = __strdup(yytext); return " + token->getGrammarToken() + "; }\n";
                break;
                case TFLOAT:
                str += token->regex + " { yylval.fval = atof(yytext); return " + token->getGrammarToken() + "; }\n";
                break;
            }
        }
        str +=  "%%\n"
                "int yywrap() { return 1; }\n";
        saveToFile(&str, "gen/" + langData->langKey + ".l");
    }
    // Generate bison grammar
    void generateGrammarFile() {
        string str = "";
        str +=  "%{\n"
                "#include <stdio.h>\n"
                "#include \"" + langData->langKey + ".hpp\"\n";
        // Result variable
        if (langData->startAction == nullptr) {
            printf("Need start key\n");
            exit(1);
        }
        langData->startAction->startPart->generateGrammarType(&str, langData);
        str += " result;\n";
        str +=  "extern FILE *yyin;\n"
                "void yyerror(const char *s);\n"
                "extern int yylex(void);\n"
                "extern int yylineno;\n"
                "%}\n";
        // Union
        str +=  "%union {\n"
                "   void *ptr;\n";
        for (TokenType ttype : langData->tokenTypes) {
            switch (ttype) {
                case TINT: str += "    int ival;\n"; break;
                case TSTRING: str += "    char *sval;\n"; break;
                case TFLOAT: str += "    double fval;\n"; break;
                case NONE: break;
            }
        }
        str += "}\n";
        // Tokens
        for (auto const &pair : langData->tokenData) {
            TokenData *token = pair.second;
            if (token->key == "WS") {
                // Used to signal whitespace needed
                // in source code
                continue;
            }
            switch (token->type) {
                case NONE: str += "%token " + token->getGrammarToken() + "\n"; break;
                case TINT: str += "%token <ival> " + token->getGrammarToken() + "\n"; break;
                case TSTRING: str += "%token <sval> " + token->getGrammarToken() + "\n"; break;
                case TFLOAT: str += "%token <fval> " + token->getGrammarToken() + "\n"; break;
            }
        }
        // Types
        // Enums goes to ival
        if (langData->enumGrammarTypes.size() > 0) {
            str += "%type <ival> ";
            for (auto const &pair : langData->enumGrammarTypes) {
                EnumGrammarType *enumGrammar = pair.second;
                str += enumGrammar->key + " ";
            }
            str += "\n";
        }
        // Ast and list goes to ptr
        if (langData->astGrammarTypes.size() > 0 || langData->listGrammarTypes.size() > 0) {
            str += "%type <ptr> ";
            str += "start ";
            for (auto const &pair : langData->astGrammarTypes) {
                AstGrammarType *astGrammar = pair.second;
                str += astGrammar->key + " ";
            }
            for (auto const &pair : langData->listGrammarTypes) {
                ListGrammarType *listGrammar = pair.second;
                str += listGrammar->key + " ";
            }
            str += "\n";
        }
        str += "%%\n";
        // Add rules
        // Start rule first
        str += "start: " + langData->startKey + " { ";
        langData->startAction->generateGrammar(&str, langData);
        str += " }\n    ;\n";
        // Ast types
        for (auto const &grammar : langData->astGrammarTypes) {
            grammar.second->generateGrammar(&str, langData);
        }
        // List types
        for (auto const &grammar : langData->listGrammarTypes) {
            grammar.second->generateGrammar(&str, langData);
        }
        // Enum types
        for (auto const &grammar : langData->enumGrammarTypes) {
            grammar.second->generateGrammar(&str, langData);
        }
        str += "\n%%\n";

        str += "void yyerror(const char *s) {\n"
               "    printf(\"Parse error on line %d: %s\", yylineno, s);\n"
               "}\n";
        saveToFile(&str, "gen/" + langData->langKey + ".y");
    }

    // Recursive to ensure parent classes
    // are added before extending classes
    void generateHeaderClass(string *str, string astClass, set<string> *addedClasses) {
        if (addedClasses->find(astClass) != addedClasses->end()) return;
        AstClass *cls = langData->astClasses[astClass];
        if (cls->extends != "") {
            generateHeaderClass(str, cls->extends, addedClasses);
        }
        cls->generateHeader(str, langData);
        addedClasses->insert(astClass);
    }

    // Generate c++ classes, enums etc
    void generateAstClasses() {
        string str = "#pragma once\n";
        str += "#include <string>\n";
        str += "#include <vector>\n";
        // Create enum with entries for each class
        str +=  "enum NodeType {\n    ";
        bool isFirst = true;
        for (auto const &astClass : langData->astClasses) {
            if (!isFirst) str += ", ";
            str += astClass.second->identifier + "Node";
            isFirst = false;
        }
        str += "\n};\n";
        for (auto const &astEnum : langData->enums) {
            astEnum.second->generateDefinition(&str, langData);
        }
        for (auto const &astEnum : langData->enums) {
            astEnum.second->generateToStringMethod(&str, langData);
        }
        // AstNode base class with nodeType
        str +=  "class AstNode {\n"
                "public:\n"
                "    NodeType nodeType;\n"
                "    AstNode(NodeType nodeType) : nodeType(nodeType) {}\n"
                "    virtual ~AstNode() {}\n"
                "};\n";
        // Forward declare classes
        for (auto const &astClass : langData->astClasses) {
            str += "class " + astClass.first + ";\n";
        }
        // Recursive method to ensure parent classes
        // are added before subclasses.
        set<string> addedClasses;
        for (auto const &astClass : langData->astClasses) {
            generateHeaderClass(&str, astClass.first, &addedClasses);
        }
        // Some externs, needed for parseFile
        str += "extern FILE *yyin;\n";
        str += "extern int yyparse();\n";
        // This extern requires ast header
        str += "extern ";
        langData->startAction->startPart->generateGrammarType(&str, langData);
        str += " result;\n";
        str += "class Loader {\npublic:\n";
        str += "static ";
        langData->startAction->startPart->generateGrammarType(&str, langData);
        str += " parseFile(std::string fileName) {\n";
        str +=  "   FILE *sourceFile;\n"
                "   #ifdef _WIN32\n"
                "   fopen_s(&sourceFile, fileName.c_str(), \"r\");\n"
                "   #else\n"
                "   sourceFile = fopen(fileName.c_str(), \"r\");\n"
                "   #endif\n"
                "   if (!sourceFile) {\n"
                "       printf(\"Can't open file %s\", fileName.c_str());\n"
                "       exit(1);\n"
                "   }\n"
                "   yyin = sourceFile;\n"
                "   do {\n"
                "       yyparse();\n"
                "   } while (!feof(yyin));\n"
                "   return result;\n"
                "}\n";
        str += "};\n";
        saveToFile(&str, "gen/" + langData->langKey + ".hpp");
    }
    void generateVisitor() {
        string *str = new string;
        *str += "#include \"" + langData->langKey + ".hpp\"\n";
        string className = langData->langKey + "Visitor";
        // Generate declaration
        *str += "class " + className + " {\n";
        *str += "public:\n";
        for (auto const &astClass : langData->astClasses) {
            *str += "   virtual void visit" + astClass.second->identifier + "(";
            *str += astClass.second->identifier + " *node);\n";
        }
        *str += "};\n";
        // Generate definitions
        for (auto const &astClass : langData->astClasses) {
            *str += "void " + className + "::visit" + astClass.second->identifier + "(";
            *str += astClass.second->identifier + " *node) {\n";
            // If this class has subclasses, pass on to more specific
            // visitor.
            // It's possibly nice to handle common members here,
            // but this would require every subclass to be passed
            // to this visitor for consistency.
            if (astClass.second->subClasses.size() > 0) {
                *str += "    switch(node->nodeType) {\n";
                for (string subClass : astClass.second->subClasses) {
                    *str += "        case " + subClass + "Node: ";
                    *str += "visit" + subClass + "(static_cast<" + subClass + "*>(node));break;\n";
                }
                *str += "        default:break;\n";
                *str += "    }\n";
            } else {
                for (auto const &member : astClass.second->members) {
                    switch (member.second->typedPart->type) {
                        case PAST: {
                            TypedPartAst *astPart = static_cast<TypedPartAst*>(member.second->typedPart);
                            AstClass *memberClass = langData->astClasses[astPart->astClass];
                            *str += "    visit" + memberClass->identifier + "(node->" + member.first + ");\n";
                        }
                        break;
                        case PLIST: {
                            // Todo list of lists
                            TypedPartList *listType = static_cast<TypedPartList*>(member.second->typedPart);
                            // Loop list of ast elements, then
                            // based on it's node type, cast it and
                            // pass to it's visitor
                            if (listType->type->type == PAST) {
                                TypedPartAst *listAstPart = static_cast<TypedPartAst*>(listType->type);
                                AstClass *listAstClass = langData->astClasses[listAstPart->astClass];
                                *str += "    for (";
                                listType->type->generateGrammarType(str, langData);
                                *str += " node : *node->" + member.first + ") {\n";
                                *str += "        visit" + listAstClass->identifier + "(node);\n";
                                *str += "    }\n";
                            }
                        }
                        break;
                        default:
                        break;
                    }
                }
            }
            *str += "}\n";
        }
        saveToFile(str, "gen/" + langData->langKey + "Visitor.hpp");
    }

    // Used to collect classes qualifying for
    // referred ast key
    void collectAstClasses(string astKey, set<string> *classes, set<string> *visited) {
        // Return if already visited
        if (visited->find(astKey) != visited->end()) return;
        visited->insert(astKey);
        AstGrammarType *grammar = langData->astGrammarTypes[astKey];
        for (AstRuleDef *ruleDef : grammar->ruleDefs) {
            if (ruleDef->refType != nullptr) {
                collectAstClasses(ruleDef->refType->identifier, classes, visited);
            } else {
                classes->insert(ruleDef->astClass);
            }
        }
    }

    void generateToSource(SourceNode *source) {
        string *str = new string;
        *str += "#include \"" + langData->langKey + "Visitor.hpp\"\n";
        *str += "#include <string>\n";
        string className = langData->langKey + "ToSource";
        *str += "class " + className + " ";
        *str += ": public " + langData->langKey + "Visitor {\n";
        *str += "public:\n";
        *str += "    std::string str;\n";
        // Gather cases in grammar
        ToSourceGenVisitor caseVisitor = ToSourceGenVisitor(langData);
        caseVisitor.visitSource(source);
        // Add declarations for key methods
        // Class methods defined in visitor base
        for (auto const &astType : langData->astGrammarTypes) {
            string astClass = astType.second->astClass;
            *str += "    void astKey_" + astType.first + "(" + astClass + " *node);\n";
        }
        for (auto const &listType : langData->listGrammarTypes) {
            *str += "    void listKey_" + listType.first + "(std::vector<";
            listType.second->type->generateGrammarType(str, langData);
            *str += "> *list);\n"; 
        }
        // Class visit decls
        for (auto const &classCase : caseVisitor.classCases) {
            *str += "    void visit" + classCase.first + "(" + classCase.first + " *node);\n";
        }
        *str += "};\n\n";
        // Go through ast types and list types
        // and generate methods to ToSource these.
        for (auto const &astType : langData->astGrammarTypes) {
            bool isClassKey = astType.first == astType.second->astClass;
            // Maybe skip those with class key
            string astClass = astType.second->astClass;
            *str += "void " + className + "::astKey_" + astType.first + "(" + astClass + " *node) {\n";
            // Switch and pass to class visitor
            *str += "    switch (node->nodeType) {\n";
            for (AstRuleDef *ruleDef : astType.second->ruleDefs) {
                if (ruleDef->refType != nullptr) {
                    set<string> *classes = new set<string>();
                    set<string> *visited = new set<string>();
                    collectAstClasses(ruleDef->refType->identifier, classes, visited);
                    for (string classCase : *classes) {
                        *str += "        case " + classCase + "Node:\n";
                    }
                    *str += "        astKey_" + ruleDef->refType->identifier + "(static_cast<" + ruleDef->refType->astClass + "*>(node));break;\n";
                } else {
                    if (caseVisitor.keyedCases[astType.first][ruleDef->astClass].size() > 1) {
                        printf("Multiple ast cases not implemented");
                        exit(1);
                    }
                    *str += "        case " + ruleDef->astClass + "Node: ";
                    *str += "visit" + ruleDef->astClass + "(static_cast<" + ruleDef->astClass + "*>(node));break;\n";
                }
            }
            *str += "    }\n}\n";
        }
        // List types
        for (auto const &listType : langData->listGrammarTypes) {
            *str += "void " + className + "::listKey_" + listType.first + "(std::vector<";
            listType.second->type->generateGrammarType(str, langData);
            *str += "> *nodes) {\n"; 
            *str += "    for (";
            listType.second->type->generateGrammarType(str, langData);
            *str += " node : *nodes) {\n";
            if (listType.second->sepBetween) {
                *str += "        if (node != nodes->front()) {\n";
                *str += "            str += \"";
                if (listType.second->sep->type == PTOKEN) {
                    TypedPartToken *sep = static_cast<TypedPartToken*>(listType.second->sep);
                    *str += sep->getCleanedVal(langData);
                } else {
                    printf("Only token separator supported\n");
                    exit(1);
                }
                *str += "\";\n        }\n";
            }
            *str += "        switch (node->nodeType) {\n";
            for (ListRuleDef *ruleDef : listType.second->ruleDefs) {
                // Generate code for each ruleDef
                string astClass;
                if (ruleDef->astRule != nullptr) {
                    astClass = ruleDef->astRule->astClass;
                    if (ruleDef->astRule->refType != nullptr) {
                        set<string> *classes = new set<string>();
                        set<string> *visited = new set<string>();
                        collectAstClasses(ruleDef->astRule->refType->identifier, classes, visited);
                        for (string classCase : *classes) {
                            *str += "            case " + classCase + "Node:\n";
                        }
                        *str += "            {\n";
                        *str += "                astKey_" + ruleDef->astRule->refType->identifier;
                        *str += "(static_cast<" + ruleDef->astRule->refType->astClass + "*>(node));\n";
                        if (ruleDef->sepAfter != nullptr) {
                            if (ruleDef->sepAfter->type == PTOKEN) {
                                TypedPartToken *sepAfter = static_cast<TypedPartToken*>(ruleDef->sepAfter);
                                *str += "                str += \"" + sepAfter->getCleanedVal(langData) + "\";\n";
                            } else {
                                printf("Only tokens supported as separators\n");
                                exit(1);
                            }
                        }
                    } else {
                        if (caseVisitor.keyedCases[listType.first][astClass].size() > 1) {
                            printf("Multiple cases not supported yet\n");
                            exit(1);
                        }
                        *str += "            case " + astClass + "Node: {\n";
                        *str += "                visit" + astClass + "(static_cast<" + astClass + "*>(node));\n";
                        if (ruleDef->sepAfter != nullptr) {
                            if (ruleDef->sepAfter->type == PTOKEN) {
                                TypedPartToken *sepAfter = static_cast<TypedPartToken*>(ruleDef->sepAfter);
                                *str += "                str += \"" + sepAfter->getCleanedVal(langData) + "\";\n";
                            } else {
                                printf("Only tokens supported as separators\n");
                                exit(1);
                            }
                        }
                    }
                }
                *str += "                break;\n            }\n";
            }
            // Close switch, for loop and function
            *str += "        }\n    }\n}\n";
        }
        // Add all classes from visitor
        for (auto const &classCase : caseVisitor.classCases) {
            *str += "void " + className + "::visit" + classCase.first + "(" + classCase.first + " *node) {\n";
            if (classCase.second.size() == 1) {
                *str += classCase.second[0]->code;
                *str += "\n";
            } else {
                // Several cases
                printf("Multiple cases todo");
                exit(1);
            }
            *str += "}\n";
        }
        saveToFile(str, "gen/" + langData->langKey + "ToSource.hpp");
    }
    void generateTransformer(){}

    /**
     * Executes command and return output
     * Returns nullptr on error status
     */
    static string* execute(string cmd) {
        std::array<char, 128> buffer;
        string *result = new string;
        FILE* pipe(popen(cmd.c_str(), "r"));
        if (!pipe) {
            printf("Pipe failed");
            exit(1);
        }
        while (!feof(pipe)) {
            if (fgets(buffer.data(), 128, pipe) != nullptr)
                *result += buffer.data();
        }
        pclose(pipe);
        printf("cmd: %s\n", cmd.c_str());
        return result;
    }

    void runFlexBison() {
        // Flex
        string lexFile = folder + "/gen/" + langData->langKey + ".l"; 
        string lexOutput = folder + "/gen/" + langData->langKey + ".yy.cpp"; 
        string *lexResult = execute("flex -o " + lexOutput + " " + lexFile);
        printf("result: %s\n", lexResult->c_str());
        // Bison
        string grammarFile = folder + "/gen/" + langData->langKey + ".y"; 
        string grammarOutput = folder + "/gen/" + langData->langKey + ".tab.h"; 
        string grammarHeader = folder + "/gen/" + langData->langKey + ".tab.cpp"; 
        string *grammarResult = execute("bison -o " + grammarOutput
            + " --defines=" + grammarHeader + " " + grammarFile);
        printf("result: %s\n", grammarResult->c_str());
    }
    static SourceNode* parseDescr(string fileName) {
        FILE *sourceFile;
        errno = 0;
    #ifdef _WIN32
        fopen_s(&sourceFile, fileName.c_str(), "r");
    #else
        sourceFile = fopen(fileName.c_str(), "r");
    #endif
        if (!sourceFile) {
            printf("Can't open file err:%d, file:%s\n", errno, fileName.c_str());
            exit(1);
        }
        yyin = sourceFile;
        do {    
            yyparse();
        } while (!feof(yyin));
        //std::string str; 
        //result->toStringF(&str, new FormatState());
        //fprintf(stdout, str.c_str());
        return static_cast<SourceNode*>(result);
    }

    /**
     * Runs the pipeline to generate files
     */
    static void genFiles(string folder, string langKey) {
        auto result = parseDescr(folder + "/" + langKey + ".lang");
        auto langData = new LData(langKey);
        auto keysVisit = new RegisterKeysVisitor(langData);
        auto listVisit = new RegisterListKeysVisitor(langData);
        auto builtInVisit = new AddBuiltInTokens(langData);
        auto ruleDefsVisit = new BuildRuleDefsVisitor(langData);
        auto rulesVisit = new BuildRulesVisitor(langData);
        auto astVisit = new BuildAstVisitor(langData);
        keysVisit->visitSource(result);
        builtInVisit->visitSource(result);
        listVisit->visitSource(result);
        ruleDefsVisit->visitSource(result);
        rulesVisit->visitSource(result);
        astVisit->visitSource(result);
        SourceGenerator *sourceGen = new SourceGenerator(langData, folder);
        sourceGen->execute("mkdir -p " + folder + "/gen");
        sourceGen->generateLexFile();
        sourceGen->generateGrammarFile();
        sourceGen->generateAstClasses();
        sourceGen->generateVisitor();
        sourceGen->generateToSource(result);
        sourceGen->generateTransformer();
        sourceGen->runFlexBison();
    }
};
}