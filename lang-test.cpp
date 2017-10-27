#include "DescrNode.hpp"
#include "LangData.hpp"
using namespace LangData;

extern FILE *yyin;
extern int yyparse();
extern DescrNode *result; 

SourceNode* parseFile(std::string fileName) {
    FILE *sourceFile;
    errno = 0;
#ifdef _WIN32
    fopen_s(&sourceFile, fileName.c_str(), "r");
#else
    sourceFile = fopen(fileName.c_str(), "r");
#endif
	if (!sourceFile) {
		printf("Can't open file %d", errno);
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

int main() {
	auto result = parseFile(std::string(PROJECT_ROOT) + "/test.lang");
	auto langData = new LData();
	auto keysVisit = new RegisterKeysVisitor(langData);
	auto listVisit = new RegisterListKeysVisitor(langData);
	auto builtInVisit = new AddBuiltInTokens(langData);
	auto rulesVisit = new BuildRulesVisitor(langData);
	auto astVisit = new BuildAstVisitor(langData);
	keysVisit->visitSource(result);
	builtInVisit->visitSource(result);
	listVisit->visitSource(result);
	rulesVisit->visitSource(result);
	astVisit->visitSource(result);
	SourceGenerator *sourceGen = new SourceGenerator(langData);
	sourceGen->generateLexFile();
	sourceGen->generateGrammarFile();
	sourceGen->generateAstClasses();
	sourceGen->generateVisitor();
	sourceGen->generateTransformer();
    return 0;
}