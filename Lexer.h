#pragma once
#include <ostream>
#include <unordered_map>
#include <string_view>
#include <memory>
#include <string>
#include <iostream>
#include <array>
#include <vector>
#include <optional>
#include <exception>

#define TAB_SIZE 4

namespace Parse {
	using Code = size_t;
	const int Eof = std::istream::traits_type::eof();

	struct Position {
		size_t line;
		size_t col;
	};

	bool operator== (const Position& lhs, const Position& rhs);
	bool operator!= (const Position& lhs, const Position& rhs);
	std::ostream& operator<< (std::ostream& os, const Position& elem);


	class LexerError : public std::runtime_error {
	public:
		using std::runtime_error::runtime_error;
	};

	struct Grammar {
		std::unordered_map<std::string, Code> key_words;
		std::unordered_map<std::string, Code> constants;
		std::unordered_map<std::string, Code> identifiers;
		std::unordered_map<Code, std::string> tokens_value;
		std::array<size_t, 255> symbols_attributes{ 10 };
		const Code identifier_code = 1001;
		const Code constant_code = 501;
	};

	class Reader {
	public:
		Reader(std::istream& input) : input(input) {}

		operator Reader() {
			return input;
		}

		operator bool() {
			return static_cast<bool>(input);
		}

		char get();
		void ignore();
		char peek();

		size_t line() const;
		size_t col() const;

		Position position() const;
	private:
		std::istream& input;
		size_t line_ = 1;
		size_t col_ = 1;
	};

	class Lexer {
	public:
		struct LexemesList {
			struct Item {
				Item(Code code, Position position, std::string_view value) : code(code), position(position), value(value) {}
				Item(Code code, Position position) : code(code), position(position) {}
				Item() = default;
				Code code;
				Position position;
				std::string_view value;

			};
			std::vector<Item> items;
			std::vector<std::string> errors;
		};

		Lexer(std::shared_ptr<Grammar> gramar, std::istream& input) : program(input), gramar(gramar) {};

		void Parse();

		const std::vector<std::string>& GetErrors() const;
		const std::vector<LexemesList::Item>& GetTokens() const;

	private:
		std::shared_ptr<Grammar> gramar;
		Reader program;
		std::optional<LexemesList> parsed_program;
		size_t row = 0;

		void Whitespace();
		void Comment();
		void Delimiter();
		void KeywordOrIdentifier();
		void Constant();

		void LeftPart(std::string& buffer);
		void RightPart(std::string& buffer);

		size_t Position() const;

		const LexemesList& List() const;
		std::vector<std::string>& Errors();
		std::vector<LexemesList::Item>& Tokens();
		LexemesList& List();
		void ThrowErr(std::string&& msg);
		void AddErr(std::string&& msg);
	};

	std::ostream& operator<<(std::ostream& os, const Lexer::LexemesList& list);
	std::ostream& operator<<(std::ostream& os, const Lexer::LexemesList::Item& item);
	bool operator==(const Lexer::LexemesList::Item& lhs, const Lexer::LexemesList::Item& rhs);
	bool operator==(const Lexer::LexemesList& lhs, const Lexer::LexemesList& rhs);
}
