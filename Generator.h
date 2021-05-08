#pragma once
#include "Parser.h"
#include <sstream>

namespace Parse {
	class GenerateError : public std::runtime_error {
	public:
		using std::runtime_error::runtime_error;
	};

	class Generator {
	public:
		Generator(std::shared_ptr<Parser::Node> tree) : tree(tree) {};

		void Generate();
		std::string GetListing() const { return assembly.str(); }
		const auto& GetIdentifiers() const { return identifiers; }
		const auto& GetIdentifiersOrder() const { return identifiers_order; }
		const auto& GetErrors() const { return errors; }
	private:
		std::stringstream assembly;
		std::shared_ptr<Parser::Node> tree;

		std::vector<std::string> errors;

		struct Identifier {
			std::string name;
			bool is_const = true;
			size_t offset = 0;
			size_t size = 0;
		};

		size_t offset = 0;
		std::unordered_map<std::string, Identifier> identifiers;
		std::vector<std::string> identifiers_order;

		std::shared_ptr<Parser::Node> FindNonTerm(const std::string& nonterm) const;
		void ParseTree(std::shared_ptr<Parser::Node> node, const std::string& nonterm) const;

		void ConstantDeclarations(std::shared_ptr<Parser::Node> node);
		size_t Constant(std::shared_ptr<Parser::Node> node);
		void Constants();
		void ProcedureIdentifier();
	};
}
