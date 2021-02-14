#pragma once
#include <ostream>
#include <unordered_map>
#include <string_view>
#include <memory>
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

	struct Gramar {
		std::unordered_map<std::string_view, Code> key_words;
		std::unordered_map<std::string_view, Code> constants;
		std::unordered_map<std::string_view, Code> identifiers;
		std::array<size_t, 255> symbols_attributes{ 10 };
		Code identifier_code = 1001;
		Code constant_code = 501;
	};

	class Lexer {
	public:
		struct LexemesList {
			struct Item {
				Code code;
				size_t position;
			};
			std::vector<Item> items;
			std::string program;
			std::vector<std::string> errors;
		};

		Lexer(std::shared_ptr<Gramar> gramar, std::istream& input) : program(input), gramar(gramar) {};

		void Parse();

		const std::vector<std::string>& Errors() const;
		const std::vector<LexemesList::Item>& Tokens() const;
		const std::string& Program() const;

	private:
		std::shared_ptr<Gramar> gramar;
		std::istream& program;
		std::optional<LexemesList> parsed_program;
		size_t row = 0;

		void Whitespace();
		void Comment();
		void Delimiter();
		void KeywordOrIdentifier();
		void Constant();

		bool LeftPart(std::string& buffer);
		bool RightPart(std::string& buffer);
		void SkipWhitespaces();
	};

	std::ostream& operator<<(std::ostream& os, const Lexer::LexemesList& list);
	std::ostream& operator<<(std::ostream& os, const Lexer::LexemesList::Item& item);
	bool operator==(const Lexer::LexemesList::Item& lhs, const Lexer::LexemesList::Item& rhs);
	bool operator==(const Lexer::LexemesList& lhs, const Lexer::LexemesList& rhs);
}
