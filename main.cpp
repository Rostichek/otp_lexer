#include <iostream>
#include <sstream>
#include <iomanip>
#include <ostream>
#include <string>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include "Lexer.h"
#include "LexerTests.h"
#include "test_runner.h"

#include "profile.h"

#include <unordered_set>

using namespace std;

void CompileProgram(istream& input, ostream& output) {
	Parse::Lexer lexer(CreateGrammar(), input);
	lexer.Parse();
	const auto& errors = lexer.GetErrors();	
	if (errors.size()) {
		output << errors.size() << " errors;\nErrors list:\n";
		for (const auto& error : errors)
			output << error << endl;
	}
	else {
		const auto& tokens = lexer.GetTokens();
		for (const auto& token : tokens) {
			output << setw(10) << token.position.line << setw(10) << token.position.col << setw(10) << token.code << "\t";
			if (token.value.empty())
				output << static_cast<char>(token.code) << endl;
			else
				output << token.value << endl;
		}
	}
}

int main(int argc, const char* argv[]) {
	std::ios::sync_with_stdio(false);
	TestRunner tr;
	RunLexerTests(tr);
	try {
		if (argc == 2) {
			string path(argv[1]);
			ifstream input(path+"\\input.sig");
			ofstream output(path + "\\generated.txt");
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
	cerr << "Usage: lexer.exe [input path]\n";
	return 5;
}

