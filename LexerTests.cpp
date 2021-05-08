#include "LexerTests.h"
#include <sstream>
#include <fstream>
#include "profile.h"

using namespace Parse;
using namespace std;

shared_ptr<Grammar> CreateGrammar() {
	auto grammar = make_shared<Grammar>();
	grammar->symbols_attributes.fill(10);
	for (size_t i = 0; i < grammar->symbols_attributes.size(); ++i) {
		if ((i >= 'A' && i <= 'Z'))
			grammar->symbols_attributes[i] = 2;
		else if (i >= '0' && i <= '9')
			grammar->symbols_attributes[i] = 1;
		if (i >= 9 && i <= 13 || (i >= 'a' && i <= 'z'))
			grammar->symbols_attributes[i] = 0;
	}
	grammar->symbols_attributes[40] = 4;
	grammar->symbols_attributes[32] = 0;
	grammar->symbols_attributes[36] = 5;
	grammar->symbols_attributes[39] = 6;
	grammar->symbols_attributes['='] = 3;
	grammar->symbols_attributes[';'] = 3;
	grammar->symbols_attributes['.'] = 3;
	grammar->symbols_attributes[40] = 4;
	grammar->key_words["PROGRAM"] = 401;
	grammar->key_words["BEGIN"] = 402;
	grammar->key_words["END"] = 403;
	grammar->key_words["CONST"] = 404;
	grammar->key_words["LOOP"] = 405;
	grammar->key_words["ENDLOOP"] = 406;
	grammar->key_words["RETURN"] = 407;
	grammar->key_words["IN"] = 408;
	return grammar;
}

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
		Parse::Generator generator(parser.GetTree());
		if (parser.GetErrors().empty()) {
			generator.Generate();
			//output << parser.RnderTree();
		}
		for (const auto& error : parser.GetErrors())
			output << error << endl;
		for (const auto& error : generator.GetErrors())
			output << error << endl;
		if (generator.GetErrors().empty()) {
			output << generator.GetListing();
			output << endl << endl;
			const auto& identifiers = generator.GetIdentifiers();
			output << setw(30) << "IDENTIFIER" << setw(10) << "TYPE"
				<< setw(10) << "OFFSET" << setw(10) << "SIZE" << endl;
			for (const auto& identifier : generator.GetIdentifiersOrder()) {
				output << setw(30) << identifiers.at(identifier).name << setw(10) << (identifiers.at(identifier).is_const ? "CONST" : "PROGRAM")
					<< setw(10) << identifiers.at(identifier).offset << setw(10) << identifiers.at(identifier).size << endl;
			}
		}
	}
}

void StartTest(const string& path) {
	ifstream input(path + "\\input.sig");
	ofstream output(path + "\\generated.txt");
	if (input.is_open() && output.is_open()) CompileProgram(input, output);
	else throw runtime_error("Bad file path: " + path);
	input.close();
	output.close();
}


void CheckTests(const string& path) {
	ifstream input(path);
	if (input.is_open()) {
		string test_path;
		while (getline(input, test_path)) {
			cout << "Test: '" + test_path + "': ";
			ifstream expected(test_path + "\\expected.txt");
			ifstream generated(test_path + "\\generated.txt");
			size_t line = 1;
			string exp_line, gen_line;
			bool OK = true;
			while (getline(expected, exp_line) && getline(generated, gen_line)) {
				if (exp_line != gen_line) {
					OK = false;
					cerr << endl << "Line " << line << ": "<< gen_line << " != " << exp_line;
				}
				line++;
			}
			if (getline(expected, exp_line) || getline(expected, gen_line)) {
				OK = false;
				cerr << endl << "Unequal number of lines";
			}
			if (OK) cerr << "OK" << endl;		
		}
	}
	else throw runtime_error("Bad file path: " + path);
	input.close();
}

void RunTests(const string& path) {
	ifstream input(path);
	if (input.is_open()) {
		string test_path;
		while (getline(input, test_path))
			StartTest(test_path);
	}
	else throw runtime_error("Bad file path: " + path);
	input.close();
}
