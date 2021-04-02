#include "Lexer.h"
#include <iomanip>
#include <iostream>
#include <string>
#include <cmath>
#include <sstream>

using namespace std;

namespace Parse {
	char Reader::get() {
		if (input.peek() == '\n') {
			line_++;
			col_ = 1;
		}
		else if (input.peek() == '\t') col_ += TAB_SIZE;
		else col_++;
		return input.get();
	}

	void Reader::ignore() {
		get();
	}

	char Reader::peek() {
		return input.peek();
	}

	size_t Reader::line() const { return line_; }

	size_t Reader::col() const { return col_; }

	Position Reader::position() const { return { line_, col_ }; }

	void Lexer::Whitespace() {
		char next;
		while (!grammar->symbols_attributes[next = program.peek()]) {
			if (next == '\n') row++;
			program.ignore();
			if (program.peek() == Eof) break;
		}
	}

	void Lexer::Comment() {
		program.ignore();
		if (program.peek() != '*') ThrowErr("Unopened comment");
		bool met_an_asterisk = false;
		char next;
		while (program) {
			next = program.get();
			if (met_an_asterisk && next == ')')
				return;
			met_an_asterisk = (next == '*');
		}
		ThrowErr("Unclosed comment");
	}

	void Lexer::Delimiter() {
		auto begin = program.position();
		Tokens().emplace_back(static_cast<Code>(program.get()), begin);
	}

	void Lexer::KeywordOrIdentifier() {
		auto begin = program.position();
		string buffer;
		char next;
		while (program) {
			next = program.peek();
			if (isalnum(next)) buffer += program.get();
			else break;
		}
		if (grammar->key_words.count(buffer)) {
			Tokens().emplace_back(grammar->key_words[buffer], begin, grammar->key_words.find(buffer)->first);
		}
		else {
			grammar->identifiers.try_emplace(buffer,
				grammar->identifier_code + grammar->identifiers.size());
			Tokens().emplace_back(grammar->identifiers[buffer], begin, grammar->identifiers.find(buffer)->first);
		}
	}

	optional<double> Lexer::LeftPart(string& buffer) {
		while (program) {
			if (isdigit(program.peek())) {
				buffer += program.peek();
				program.get();
			}
			else {
				char next = program.peek();
				if (!grammar->symbols_attributes[next] || next == '\'') 
					if (buffer.size()) return atoi(buffer.c_str());
					else return {};
				ThrowErr("Wrong left part");
			}
		}
	}

	optional<double> Lexer::RightPart(string& buffer) {
		if (isdigit(program.peek())) {
			return LeftPart(buffer);
		}
		string local_buffer;
		string digit;
		while (program) {
			if (program.peek() == '(') {
				program.ignore();
				if (local_buffer != "$EXP") 
					ThrowErr("Wrong right part : unknown word " + local_buffer);
				local_buffer += '(';
				break;
			}
			local_buffer += program.get();
		}
		Whitespace();
		while (program) {
			if (isdigit(program.peek())) {
				digit += program.peek();
				local_buffer += program.get();
			}
			else {
				Whitespace();
				if (program.peek() == ')') {
					buffer += local_buffer + ')';
					program.ignore();
					return exp(atoi(digit.c_str()));
				}
				ThrowErr("Wrong right part : unclosed exponent body");
			}
		}
	}

	void Lexer::AddErr(std::string&& msg) {
		Errors().emplace_back(msg);
	}

	void Lexer::ThrowErr(std::string&& msg) {
		throw LexerError{ "Lexer: Error (line " + to_string(program.line()) +
			", col " + to_string(program.col() - 1) + ") : " + msg + ";" };
	}

	void Lexer::Constant() {
		auto begin = program.position();
		Complex complex;
		string left;
		string right;
		program.get();
		bool error = 0;
		Whitespace();
		try {
			complex.left = LeftPart(left);
		}
		catch (LexerError& ex) {
			error = true;
			AddErr(ex.what());
		}
		Whitespace();
		if (program.peek() != '\'' && !error) {
			try {
				complex.right = RightPart(right);
			}
			catch (LexerError& ex) {
				error = true;
				AddErr(ex.what());
			}
		}
		while (program && program.get() != '\'');
		if (!program) ThrowErr("Unclosed constant");
		if (error) return;
		if (right.size()) left += ' ';
		string buffer = '\'' + left + right + '\'';
		grammar->constants.try_emplace(buffer,
			grammar->constant_code + grammar->constants.size());
		Tokens().emplace_back(grammar->constants[buffer], begin, grammar->constants.find(buffer)->first);
		Tokens().back().complex = complex;
	}
	
	void Lexer::Parse() {
		if (parsed_program.has_value()) return;
		parsed_program.emplace();
		while(program.peek() != Eof) {
			char next = program.peek();
			try {
				switch (grammar->symbols_attributes[next]) {
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
					stringstream out;
					out << "Illegal symbol \'" << next << "\'";
					ThrowErr(out.str());
				}
			}
			catch (LexerError& ex) {
				AddErr(ex.what());
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

	Lexer::LexemesList& Lexer::List() {
		if (parsed_program.has_value())
			return parsed_program.value();
		throw bad_optional_access();
	}

	std::vector<std::string>& Lexer::Errors() { return List().errors; }

	std::vector<Lexer::LexemesList::Item>& Lexer::Tokens() { return List().items; }

	bool operator==(const Lexer::LexemesList::Item& lhs, const Lexer::LexemesList::Item& rhs) {
		if (lhs.code != rhs.code) return false;
		if (lhs.position != rhs.position) return false;
		return true;
	}

	bool operator==(const Lexer::LexemesList& lhs, const Lexer::LexemesList& rhs) {
		if (lhs.items != rhs.items) return false;
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

	bool operator== (const Position& lhs, const Position& rhs) {
		return tie(lhs.line, lhs.col) == tie(rhs.line, rhs.col);
	}

	bool operator!= (const Position& lhs, const Position& rhs) {
		return !(lhs == rhs);
	}

	ostream& operator<< (ostream& os, const Position& elem) {
		os << "[" << elem.line << "; " << elem.col << "]";
		return os;
	}
}