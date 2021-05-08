#pragma once
#include "Lexer.h"

namespace Parse {
	using LexemeIt = std::vector<Parse::Lexer::LexemesList::Item>::const_iterator;

	class ParserError : public std::runtime_error {
	public:
		using std::runtime_error::runtime_error;
	};

	class Parser {
	public:
		Parser(std::shared_ptr<Grammar> grammar, 
			const std::vector<Parse::Lexer::LexemesList::Item>& lexemes)
			: lexemes_list(lexemes), grammar(grammar), lexeme(lexemes_list.begin())
		{};

		void Parse();
		std::string RnderTree();
		const std::vector<std::string>& GetErrors() const;

		auto GetTree() { return tree; }

		struct Node {
			Node(std::string not_term) : not_term(not_term) {};
			Node(LexemeIt term) : term(term) {};

			std::optional<std::string> not_term;
			std::optional<LexemeIt> term;
			std::vector<std::shared_ptr<Node>> children;
		};

	private:
		std::shared_ptr<Node> tree = std::make_unique<Node>("<signal_program>");
		std::shared_ptr<Grammar> grammar;
		const std::vector<Parse::Lexer::LexemesList::Item>& lexemes_list;
		LexemeIt lexeme;

		std::vector<std::string> errors;
		
		void ComputeRender(std::stringstream&, std::shared_ptr<Node>, size_t deep = 0);

		void Scan();
		LexemeIt GetLexeme();
		void ThrowErr(std::string&& expected, LexemeIt found);

		std::shared_ptr<Node> Program();
		std::shared_ptr<Node> ProcedureIdentifier();
		std::shared_ptr<Node> ConstantIdentifier();
		std::shared_ptr<Node> Block();
		std::shared_ptr<Node> Declarations();
		std::shared_ptr<Node> StatementsList();
		std::shared_ptr<Node> Statement();
		std::shared_ptr<Node> ConstantDeclarations();
		std::shared_ptr<Node> ConstantDeclarationsList();
		std::shared_ptr<Node> ConstantDeclaration();
		std::shared_ptr<Node> Constant();
		std::shared_ptr<Node> Empty();
		std::shared_ptr<Node> Identifier();

	};
}

