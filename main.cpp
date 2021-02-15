#include <iostream>
#include <sstream>
#include <iomanip>
#include <ostream>
#include <string>
#include <fstream>
#include "Lexer.h"
#include "LexerTests.h"
#include "test_runner.h"

#include "profile.h"

using namespace std;

void CompileProgram(istream& input, ostream& output) {
	Parse::Lexer lexer(CreateGrammar(), input);
	lexer.Parse();
	const auto& errors = lexer.GetErrors();	
	if (errors.size()) {
		cerr << errors.size() << "errors;\nErrors list:\n";
		for (const auto& error : errors)
			cerr << error << endl;
	}
	output << "Lexemes list: " << endl;
	const auto& tokens = lexer.GetTokens();
	output << tokens;
}

int main(int argc, const char* argv[]) {
	std::ios::sync_with_stdio(false);
	TestRunner tr;
	RunLexerTests(tr);
	try {
		if (argc == 3) {
			ifstream input(argv[1]);
			ofstream output(argv[2]);
			if (input.is_open() && output.is_open()) CompileProgram(input, output);
			else throw runtime_error("Bad file path.");
			input.close();
			output.close();
			return 0;
		}
	}
	catch (exception& ex) {
		cerr << ex.what();
		return 1;
	}
	cerr << "Usage: lexer.exe [input path] [output path]\n";
	return 5;
}

