#include <iostream>
#include <sstream>
#include <iomanip>
#include <ostream>
#include <string>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include "Lexer.h"
#include "Parser.h"
#include "LexerTests.h"
#include "test_runner.h"
#include <unordered_set>

using namespace std;

void CompileProgram(istream& input, ostream& output) {
	auto grammar = CreateGrammar();
	Parse::Lexer lexer(grammar, input);
	lexer.Parse();
	const auto& errors = lexer.GetErrors();	
	if (errors.size()) {
		output << errors.size() << " lexer errors was found;\n";
		for (const auto& error : errors)
			output << error << endl;
	}
	else {
		// Lexer listing
		//const auto& tokens = lexer.GetTokens();
		//for (const auto& token : tokens) {
		//	output << setw(10) << token.position.line << setw(10) << token.position.col << setw(10) << token.code << "\t";
		//	if (token.value.empty())
		//		output << static_cast<char>(token.code) << endl;
		//	else
		//		output << token.value << endl;
		//}

		Parse::Parser parser(grammar, lexer.GetTokens());
		parser.Parse();
		const auto& errors = parser.GetErrors();
		if(errors.empty())
			output << parser.RnderTree();
		for (const auto& error : errors)
			output << error << endl;
	}
}

void StartTest(const string& path) {
	ifstream input(path + "\\input.sig");
	ofstream output(path + "\\generated.txt");
	if (input.is_open() && output.is_open()) CompileProgram(input, output);
	else throw runtime_error("Bad file path.");
	input.close();
	output.close();
}

int main(int argc, const char* argv[]) {
	std::ios::sync_with_stdio(false);
	if (argc == 3 && string(argv[2]) == "-d") {
		TestRunner tr;
		RunLexerTests(tr);
	}
	try {
		if (argc > 1) {
			StartTest(argv[1]);
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

