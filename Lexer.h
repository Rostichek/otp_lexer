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

namespace Parse {
	using Code = size_t;
	const int Eof = std::istream::traits_type::eof();

	class LexerError : public std::runtime_error {
	public:
		using std::runtime_error::runtime_error;
	};

	struct Grammar {
		std::unordered_map<std::string, Code> key_words;
		std::unordered_map<std::string_view, Code> constants;
		std::unordered_map<std::string_view, Code> identifiers;
		std::array<size_t, 255> symbols_attributes{ 10 };
		const Code identifier_code = 1001;
		const Code constant_code = 501;
	};

	class Lexer {
	public:
		struct LexemesList {
			struct Item {
				Item(Code code, size_t position) : code(code), position(position) {};
				Item() = default;
				Code code;
				size_t position;
			};
			std::vector<Item> items;
			std::string program;
			std::vector<std::string> errors;
		};

		Lexer(std::shared_ptr<Grammar> gramar, std::istream& input) : program(input), gramar(gramar) {};

		void Parse();

		const std::vector<std::string>& GetErrors() const;
		const std::vector<LexemesList::Item>& GetTokens() const;
		const std::string& GetProgram() const;

	private:
		std::shared_ptr<Grammar> gramar;
		std::istream& program;
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
		void InitParse();

		const LexemesList& List() const;
		std::vector<std::string>& Errors();
		std::vector<LexemesList::Item>& Tokens();
		std::string& Program();
		LexemesList& List();
	};

	std::ostream& operator<<(std::ostream& os, const Lexer::LexemesList& list);
	std::ostream& operator<<(std::ostream& os, const Lexer::LexemesList::Item& item);
	bool operator==(const Lexer::LexemesList::Item& lhs, const Lexer::LexemesList::Item& rhs);
	bool operator==(const Lexer::LexemesList& lhs, const Lexer::LexemesList& rhs);
}
