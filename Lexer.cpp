#include "Lexer.h"
#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

namespace Parse {
	void Lexer::SkipWhitespaces() {
		while (!gramar->symbols_attributes[program.peek()]) {
			if (program.peek() == '\n') 
				row++;
			program.get();
			if (program.peek() == Eof) break;
		}
	}

	void Lexer::Whitespace() {
		SkipWhitespaces();
	}

	void Lexer::Comment() {
		program.get();
		if (program.peek() != '*') throw LexerError{ "Unopened comment;" };
		bool met_an_asterisk = false;
		char next;
		while (1) {
			next = program.get();
			if (!program) break;
			if (met_an_asterisk && next == ')')
				return;
			met_an_asterisk = (next == '*');
		}
		throw LexerError{ "Unclosed comment;" };
	}

	void Lexer::Delimiter() {
		parsed_program.value().items.push_back({ static_cast<Code>(program.peek()), parsed_program.value().program.size() });
		parsed_program.value().program.push_back(program.get());
	}

	void Lexer::KeywordOrIdentifier() {
		string buffer;
		while (program) {
			if (isdigit(program.peek()) || isalpha(program.peek())) {
				buffer += program.get();
			}
			else break;
		}
		if (gramar->key_words.count(buffer)) parsed_program.value().items.push_back({ gramar->key_words[buffer], parsed_program.value().program.size() });
		else {
			if(!gramar->identifiers.count(buffer))
				gramar->identifiers[buffer] = gramar->identifier_code++;
			parsed_program.value().items.push_back({ gramar->identifiers[buffer], parsed_program.value().program.size() });
		}
		parsed_program.value().program += buffer;
	}

	bool Lexer::LeftPart(string& buffer) {
		while (program) {
			if (isdigit(program.peek())) {
				buffer += program.peek();
				program.get();
			}
			else {
				if (!gramar->symbols_attributes[program.peek()] || program.peek() == '\'') return true;
				throw LexerError{ "Wrong left part" };
			}
		}
	}

	bool Lexer::RightPart(string& buffer) {
		if (isdigit(program.peek())) {
			LeftPart(buffer);
			return true;
		}
		string local_buffer;
		while (program) {
			if (program.peek() == '(') {
				program.get();
				if (local_buffer != "$EXP") 
					throw LexerError{""};
				local_buffer += '(';
				break;
			}
			local_buffer += program.peek();
			program.get();
		}
		if (!gramar->symbols_attributes[program.peek()]) SkipWhitespaces();
		while (program) {
			if (isdigit(program.peek())) {
				local_buffer += program.peek();
				program.get();
			}
			else {
				if (!gramar->symbols_attributes[program.peek()]) SkipWhitespaces();
				if (program.peek() == ')') {
					program.get();
					buffer += local_buffer + ')';
					return true;
				}
				throw LexerError{ "Wrong right part" };
			}
		}
	}

	void Lexer::Constant() {
		string buffer;
		program.get();
		if (!gramar->symbols_attributes[program.peek()]) SkipWhitespaces();
		try {
			LeftPart(buffer);
		}
		catch (LexerError& ex) {
			throw LexerError{ "Wrong left part;" };
		}
		if (!gramar->symbols_attributes[program.peek()]) SkipWhitespaces();
		if (program.peek() != '\'') {
			try {
				RightPart(buffer);
			}
			catch (LexerError& ex) {
				throw LexerError{ "Wrong right part;" };
			}
		}
		if (!gramar->symbols_attributes[program.peek()]) SkipWhitespaces();
		if(program.get() != '\'') throw LexerError{ "Unclosed constant;" };
		if(!gramar->constants.count(buffer)) gramar->constants[buffer] = gramar->constant_code++;
		parsed_program.value().items.push_back({ gramar->constants[buffer], parsed_program.value().program.size() });
		parsed_program.value().program += '\'' + buffer + '\'';
	}

	void Lexer::Parse() {
		if (parsed_program.has_value()) return;
		parsed_program.emplace();
		while(program.peek() != Eof) {
			try {
				if (!gramar->symbols_attributes[program.peek()])
					Whitespace();
				else if (gramar->symbols_attributes[program.peek()] == 4)
					Comment();
				else if (gramar->symbols_attributes[program.peek()] == 3)
					Delimiter();
				else if (gramar->symbols_attributes[program.peek()] == 2)
					KeywordOrIdentifier();
				else if (gramar->symbols_attributes[program.peek()] == 6)
					Constant();
				else {
					program.get();
					throw LexerError{ "Unknown symbol" };
				}
			}
			catch (LexerError& ex) {
				parsed_program.value().errors.push_back("Lexical error in " + to_string(row) + " row : " + ex.what());
			}
		}
	}

	const std::vector<std::string>& Lexer::Errors() const {
		if (parsed_program.has_value())
			return parsed_program.value().errors;
		throw bad_optional_access();
	}

	const std::vector<Lexer::LexemesList::Item>& Lexer::Tokens() const {
		if (parsed_program.has_value())
			return parsed_program.value().items;
		throw bad_optional_access();
	}

	const std::string& Lexer::Program() const {
		if (parsed_program.has_value())
			return parsed_program.value().program;
		throw bad_optional_access();
	}



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