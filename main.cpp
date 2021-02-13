#include <iostream>
#include <sstream>
#include <iomanip>
#include <ostream>
#include <string>
#include "Lexer.h"
#include "LexerTests.h"
#include "test_runner.h"

using namespace std;

void CompileProgram(string_view program) {
	Parse::Lexer lexer(CreateGramar());
	auto tokens = lexer.Parse(program);
	cerr << tokens.errors.size() << "errors;\nErrors list:\n";
	for (const auto& error : tokens.errors)
		cerr << error << endl;
	cout << "Lexemes list: " << endl;
	cout << tokens;
}

string ReadProgram(istream& is) {
	string program;
	string tmp;
	while (getline(is, tmp)) program += tmp + "\n";
	return program;
}

int main() {
	TestRunner tr;
	RunLexerTests(tr);
	CompileProgram(ReadProgram(cin));
	return 0;
}

