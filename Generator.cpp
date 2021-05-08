#include "Generator.h"
#include <cmath>

using namespace std;
using namespace Parse;

void Generator::ParseTree(shared_ptr<Parser::Node> node, const string& nonterm) const {
	if (node->not_term.has_value())
		if (node->not_term == nonterm) throw node;
	for (auto& child : node->children)
		ParseTree(child, nonterm);
}


shared_ptr<Parser::Node> Generator::FindNonTerm(const string& nonterm) const {
	try {
		ParseTree(tree, nonterm);
	}
	catch (shared_ptr<Parser::Node> node) {
		return node;
	}
	return {};
}

void Generator::ProcedureIdentifier() {
	auto procedure_identifier = FindNonTerm("<procedure-identifier>");
	string identifier(procedure_identifier->children.front()->children.front()->term.value()->value); // Procedure name;
	identifiers[identifier] = Identifier{ identifier, false };
	identifiers_order.emplace_back(identifier);
}

size_t Generator::Constant(shared_ptr<Parser::Node> node) {
	auto complex = node->term.value()->complex.value();
	if (complex.left.has_value() && complex.exp.has_value()) {
		offset += 8;
		assembly << "mov QWORD PTR[rbp - " + to_string(offset) + "], " +
			to_string(static_cast<uint64_t>(complex.left.value() * pow(10, complex.exp.value()))) +"\n";
		return 8;
	}
	else if (complex.left.has_value() && !complex.right.has_value()) {
		offset += 8;
		assembly << "mov QWORD PTR[rbp - " + to_string(offset) + "], " +
			to_string(complex.left.value()) + "\n";
		return 8;
	}
	if (complex.right.has_value()) {
		offset += 8;
		assembly << "mov QWORD PTR[rbp - " + to_string(offset + 8) + "], " + to_string(complex.right.value()) + "\n";
		assembly << "mov QWORD PTR[rbp - " + to_string(offset) + "], " + to_string(complex.left.value()) + "\n";
		offset += 8;
		return 16;
	}
	return 0;
}

void Generator::ConstantDeclarations(shared_ptr<Parser::Node> node) {
	auto lexeme = node->children.front()->children.front()->children.front()->term.value();
	string identifier(lexeme->value); // Const identifier;
	if (identifiers.count(identifier)) throw GenerateError("Code Generator: Error (line " + to_string(lexeme->position.line)
		+ ", column " + to_string(lexeme->position.col) + "): The constant name '" + identifier + "' is used a second time;");
	assembly << "; " << identifier << endl;
	identifiers[identifier] = Identifier{ identifier, true, offset,
		Constant(node->children[2]->children.front()) };
	identifiers_order.emplace_back(identifier);
	return;
}

void Generator::Constants() {
	auto constants = FindNonTerm("<constant-declarations>");
	if (constants->children.size() == 1) // -> empty
		return;
	try {
		auto declarations_list = constants->children[1];
		while (declarations_list->children.size() > 1) {
			ConstantDeclarations(declarations_list->children.front());
			declarations_list = declarations_list->children.back();
		}
	}
	catch (GenerateError& err) {
		errors.push_back(err.what());
	}
}

void Generator::Generate() {
	assembly << "push rbp\nmov rbp, rsp\n";
	ProcedureIdentifier();
	Constants();
	assembly << "pop rbp\nret";
}