#include "LexerTests.h"

using namespace std;

int main(int argc, const char* argv[]) {
	std::ios::sync_with_stdio(false);
	//StartTest("..\\Debug\\tests\\test_max");
	//return 1;
	try {
		if (argc > 1) {
			if (string(argv[1]) == "-d") {
				RunTests("..\\Debug\\tests\\tests.txt");
				CheckTests("..\\Debug\\tests\\tests.txt");
			}
			else StartTest(argv[1]);
			return 0;
		}
	}
	catch (exception& ex) {
		cerr << ex.what();
		return 1;
	}
	cerr << "Usage: lexer.exe [input path]\n";
	return 5;
}

