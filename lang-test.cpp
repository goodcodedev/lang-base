#include "DescrNode.hpp"

extern FILE *yyin;
extern int yyparse();
extern DescrNode *result; 

DescrNode* parseFile(std::string fileName) {
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
	return result;
}

int main() {
    auto result = parseFile(std::string(PROJECT_ROOT) + "/test.lang");
    return 0;
}