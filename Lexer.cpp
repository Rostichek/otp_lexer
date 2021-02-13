#include "Lexer.h"
#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

namespace Parse {
	void Lexer::SkipWhitespaces(string_view program, size_t& cur) {
		while (!gramar->symbols_attributes[program[cur]]) {
			if (program[cur] == '\n') 
				row++;
			if (cur == program.size() - 1) break;
			cur++;
		}
	}

	void Lexer::Whitespace(string_view program, size_t& cur, LexemesList& list) {
		SkipWhitespaces(program, cur);
		if (cur != program.size() - 1) cur--;
	}

	void Lexer::Comment(string_view program, size_t& cur, LexemesList& list) {
		if (program[++cur] != '*') throw LexerError{ "Unopened comment;" };
		bool met_an_asterisk = false;
		while (++cur < program.size()) {
			if (met_an_asterisk && program[cur] == ')')
				return;
			met_an_asterisk = (program[cur] == '*');
		}
		throw LexerError{ "Unclosed comment;" };
	}

	void Lexer::Delimiter(string_view program, size_t& cur, LexemesList& list) {
		list.items.push_back({ static_cast<Code>(program[cur]), list.program.size() });
		list.program.push_back(program[cur]);
	}

	void Lexer::KeywordOrIdentifier(string_view program, size_t& cur, LexemesList& list) {
		string buffer;
		while (cur < program.size()) {
			if (isdigit(program[cur]) || isalpha(program[cur])) {
				buffer += program[cur];
				cur++;
			}
			else break;
		}
		if (gramar->key_words.count(buffer)) list.items.push_back({ gramar->key_words[buffer], list.program.size() });
		else {
			gramar->identifiers[buffer] = gramar->identifier_code++;
			list.items.push_back({ gramar->identifiers[buffer], list.program.size() });
		}
		list.program += buffer;
		if (cur != program.size() - 1) cur--;
	}

	bool Lexer::LeftPart(string& buffer, string_view program, size_t& cur) {
		while (cur < program.size()) {
			if (isdigit(program[cur])) {
				buffer += program[cur];
				cur++;
			}
			else {
				if (!gramar->symbols_attributes[program[cur]] || program[cur] == '\'') return true;
				throw LexerError{ "Wrong left part" };
			}
		}
	}

	bool Lexer::RightPart(string& buffer, string_view program, size_t& cur) {
		if (isdigit(program[cur])) {
			LeftPart(buffer, program, cur);
			return true;
		}
		string local_buffer;
		while (cur < program.size()) {
			if (program[cur] == '(') {
				cur++;
				if (local_buffer != "$EXP") 
					throw LexerError{};
				local_buffer += '(';
				break;
			}
			local_buffer += program[cur];
			cur++;
		}
		if (!gramar->symbols_attributes[program[cur]]) SkipWhitespaces(program, cur);
		while (cur < program.size()) {
			if (isdigit(program[cur])) {
				local_buffer += program[cur];
				cur++;
			}
			else {
				if (!gramar->symbols_attributes[program[cur]]) SkipWhitespaces(program, cur);
				if (program[cur] == ')') {
					cur++;
					buffer += local_buffer + ')';
					return true;
				}
				throw LexerError{ "Wrong right part" };
			}
		}
	}

	void Lexer::Constant(string_view program, size_t& cur, LexemesList& list) {
		string buffer;
		cur++;
		if (!gramar->symbols_attributes[program[cur]]) SkipWhitespaces(program, cur);
		try {
			LeftPart(buffer, program, cur);
		}
		catch (LexerError& ex) {
			throw LexerError{ "Wrong left part;" };
		}
		if (!gramar->symbols_attributes[program[cur]]) SkipWhitespaces(program, cur);
		if (program[cur] != '\'') {
			try {
				RightPart(buffer, program, cur);
			}
			catch (LexerError& ex) {
				throw LexerError{ "Wrong right part;" };
			}
		}
		if (!gramar->symbols_attributes[program[cur]]) SkipWhitespaces(program, cur);
		if(program[cur] != '\'') throw LexerError{ "Unclosed constant;" };
		gramar->constants[buffer] = gramar->constant_code++;
		list.items.push_back({ gramar->constants[buffer], list.program.size() });
		list.program += '\'' + buffer + '\'';
	}

	Lexer::LexemesList Lexer::Parse(string_view program) {
		LexemesList result;
		size_t pos = 0;
		for (size_t i = 0; i < program.size(); ++i) {
			try {
				if (!gramar->symbols_attributes[program[i]])
					Whitespace(program, i, result);
				else if (gramar->symbols_attributes[program[i]] == 4)
					Comment(program, i, result);
				else if (gramar->symbols_attributes[program[i]] == 3)
					Delimiter(program, i, result);
				else if (gramar->symbols_attributes[program[i]] == 2)
					KeywordOrIdentifier(program, i, result);
				else if (gramar->symbols_attributes[program[i]] == 6)
					Constant(program, i, result);
				else throw LexerError{ "Unknown symbol" };
				pos = result.program.size() - 1;
			}
			catch (LexerError& ex) {
				result.errors.push_back("Lexical error in " + to_string(row) + " row : " + ex.what);
			}
		}
		return result;
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
}