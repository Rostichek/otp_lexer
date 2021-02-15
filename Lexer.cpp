#include "Lexer.h"
#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

namespace Parse {
	void Lexer::Whitespace() {
		while (!gramar->symbols_attributes[program.peek()]) {
			if (program.peek() == '\n') row++;
			program.ignore();
			if (program.peek() == Eof) break;
		}
	}

	void Lexer::Comment() {
		program.ignore();
		if (program.peek() != '*') throw LexerError{ "Unopened comment;" };
		bool met_an_asterisk = false;
		char next;
		while (program) {
			next = program.get();
			if (met_an_asterisk && next == ')')
				return;
			met_an_asterisk = (next == '*');
		}
		throw LexerError{ "Unclosed comment;" };
	}

	void Lexer::Delimiter() {
		Tokens().emplace_back(static_cast<Code>(program.peek()), Position());
		Program() += program.get();
	}

	void Lexer::KeywordOrIdentifier() {
		string buffer;
		char next;
		while (program) {
			next = program.peek();
			if (isalnum(next)) buffer += program.get();
			else break;
		}
		size_t begin = Position();
		Program() += buffer;
		if (gramar->key_words.count(buffer)) 
			Tokens().emplace_back(gramar->key_words[buffer], begin);
		else {
			gramar->identifiers.try_emplace(string_view{ &Program().c_str()[begin], buffer.size() },
				gramar->identifier_code + gramar->identifiers.size());
			Tokens().emplace_back(gramar->identifiers[buffer], begin);
		}
	}

	size_t Lexer::Position() const { return GetProgram().size(); }

	void Lexer::LeftPart(string& buffer) {
		while (program) {
			if (isdigit(program.peek())) {
				buffer += program.peek();
				program.get();
			}
			else {
				char next = program.peek();
				if (!gramar->symbols_attributes[next] || next == '\'') return;
				throw LexerError{ "Wrong left part" };
			}
		}
	}

	void Lexer::RightPart(string& buffer) {
		if (isdigit(program.peek())) {
			LeftPart(buffer);
			return;
		}
		string local_buffer;
		while (program) {
			if (program.peek() == '(') {
				program.ignore();
				if (local_buffer != "$EXP") 
					throw LexerError{""};
				local_buffer += '(';
				break;
			}
			local_buffer += program.get();
		}
		Whitespace();
		while (program) {
			if (isdigit(program.peek()))
				local_buffer += program.get();
			else {
				Whitespace();
				if (program.get() == ')') {
					buffer += local_buffer + ')';
					return;
				}
				throw LexerError{ "Wrong right part" };
			}
		}
	}

	void Lexer::Constant() {
		string buffer;
		program.get();
		bool error = 0;
		Whitespace();
		try {
			LeftPart(buffer);
		}
		catch (LexerError& ex) {
			error = true;
			parsed_program.value().errors.emplace_back("Lexical error in " + to_string(row) + " row : " + "Wrong left part;");
		}
		Whitespace();
		if (program.peek() != '\'') {
			try {
				RightPart(buffer);
			}
			catch (LexerError& ex) {
				error = true;
				parsed_program.value().errors.emplace_back("Lexical error in " + to_string(row) + " row : " + "Wrong right part;");
			}
		}
		while (program && program.get() != '\'');
		if (!program) throw LexerError{ "Unclosed constant;" };
		if (error) return;
		size_t begin = Position();
		Program() += '\'' + buffer + '\'';
		gramar->constants.try_emplace(string_view{ &Program().c_str()[begin+1], buffer.size() },
			gramar->constant_code + gramar->constants.size());
		Tokens().push_back({ gramar->constants[buffer], begin });
	}

	void Lexer::InitParse() {
		parsed_program.emplace();
		program.seekg(0, ios::end);
		parsed_program.value().program.reserve(static_cast<size_t>(program.tellg()) + 1);
		program.seekg(0);
	}

	void Lexer::Parse() {
		if (parsed_program.has_value()) return;
		InitParse();
		while(program.peek() != Eof) {
			try {
				switch (gramar->symbols_attributes[program.peek()]) {
				case 0:
					Whitespace();
					break;
				case 2:
					KeywordOrIdentifier();
					break;
				case 3:
					Delimiter();
					break;
				case 4:
					Comment();
					break;
				case 6:
					Constant();
					break;
				default:
					program.ignore();
					throw LexerError{ "Unknown symbol" };
					break;
				}
			}
			catch (LexerError& ex) {
				parsed_program.value().errors.emplace_back("Lexical error in " + to_string(row) + " row : " + ex.what());
			}
		}
	}

	const Lexer::LexemesList &Lexer::List() const {
		if (parsed_program.has_value())
			return parsed_program.value();
		throw bad_optional_access();
	}

	const std::vector<std::string>& Lexer::GetErrors() const { return List().errors; }

	const std::vector<Lexer::LexemesList::Item>& Lexer::GetTokens() const { return List().items; }

	const std::string& Lexer::GetProgram() const { return List().program; }

	Lexer::LexemesList& Lexer::List() {
		if (parsed_program.has_value())
			return parsed_program.value();
		throw bad_optional_access();
	}

	std::vector<std::string>& Lexer::Errors() { return List().errors; }

	std::vector<Lexer::LexemesList::Item>& Lexer::Tokens() { return List().items; }

	std::string& Lexer::Program() { return List().program; }

	bool operator==(const Lexer::LexemesList::Item& lhs, const Lexer::LexemesList::Item& rhs) {
		if (lhs.code != rhs.code) return false;
		if (lhs.position != rhs.position) return false;
		return true;
	}

	bool operator==(const Lexer::LexemesList& lhs, const Lexer::LexemesList& rhs) {
		if (lhs.items != rhs.items) return false;
		if (lhs.program != rhs.program) return false;
		return true;
	}

	std::ostream& operator<<(std::ostream& os, const Lexer::LexemesList& list) {
		os << endl;
		for (const auto& item : list.items)
			os << setw(5) << item.code;
		os << endl;
		for (const auto& item : list.items)
			os << setw(5) << item.position;
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const Lexer::LexemesList::Item& item) {
		os << item.code << " on position " << item.position;
		return os;
	}
}