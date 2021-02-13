#pragma once
#include <ostream>
#include <unordered_map>
#include <string_view>
#include <memory>
#include <array>
#include <vector>

namespace Parse {
	using Code = size_t;
	struct LexerError {
		std::string what;
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

		Lexer(std::shared_ptr<Gramar> gramar) : gramar(gramar) {};

		LexemesList Parse(std::string_view program);
	private:
		std::shared_ptr<Gramar> gramar;
		size_t row;

		void Whitespace(std::string_view program, size_t& cur, LexemesList& list);
		void Comment(std::string_view program, size_t& cur, LexemesList& list);
		void Delimiter(std::string_view program, size_t& cur, LexemesList& list);
		void KeywordOrIdentifier(std::string_view program, size_t& cur, LexemesList& list);
		void Constant(std::string_view program, size_t& cur, LexemesList& list);

		bool LeftPart(std::string& buffer, std::string_view program, size_t& cur);
		bool RightPart(std::string& buffer, std::string_view program, size_t& cur);
		void SkipWhitespaces(std::string_view program, size_t& cur);

	};

	std::ostream& operator<<(std::ostream& os, const Lexer::LexemesList& list);
	bool operator==(const Lexer::LexemesList::Item& lhs, const Lexer::LexemesList::Item& rhs);
	bool operator==(const Lexer::LexemesList& lhs, const Lexer::LexemesList& rhs);
}
