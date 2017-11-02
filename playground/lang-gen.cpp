#include <LangBase/Process/SourceGenerator.hpp>
using namespace LangBase;

int main() {
	SourceGenerator::genFiles(string(PROJECT_ROOT) + "/playground", "TestLang");
    return 0;
}