#include "TestLang.hpp"
#include "TestLangToSource.hpp"
#include <string>

using std::string;

int main() {
	string testFile = string(PROJECT_ROOT) + "/playground/lang.test";
	auto result = Loader::parseFile(testFile);
	auto toSource = TestLangToSource();
	toSource.visitFunction(result);
	return 0;
}