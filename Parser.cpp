#include "Parser.h"
#include <sstream>
#include <iomanip>

using namespace std;
using namespace Parse;

void Parser::Scan() {
	if (lexeme != lexemes_list.end())
		lexeme++;
}

LexemeIt Parser::GetLexeme() {
	if (lexeme != lexemes_list.end()) return lexeme;
	throw ParserError("Parser: Error: The end of the program was found earlier than expected;");
}


const std::vector<std::string>& Parser::GetErrors() const {
	return errors;
}

string Parser::RnderTree() {
	std::stringstream rendered_tree;
	ComputeRender(rendered_tree, tree, 0);
	return rendered_tree.str();
}

void Parser::ComputeRender(std::stringstream& render, std::shared_ptr<Node> node, size_t deep) {
	render << setfill('.') << setw(deep * 2) << "";
	render << node->not_term.value_or("");
	if (node->term.has_value()) {
		render << node->term.value()->code << " ";
		if (node->term.value()->value.empty())
			render << static_cast<char>(node->term.value()->code);
		else
			render << node->term.value()->value;
	}
	render << '\n';
	for (auto& child : node->children)
		ComputeRender(render, child, deep + 1);
}

void Parser::ThrowErr(string&& expected, LexemeIt found) {
	throw ParserError("Parser: Error (line " + 
		to_string(GetLexeme()->position.line) + ", column " + to_string(GetLexeme()->position.col) +
		"): ‘" + expected + "’ expected but ‘" + (found->value.size() ? string(found->value) : (string() + static_cast<char>(found->code))) + "’ found;");
}

void Parser::Parse() {
	try {
		tree->children.push_back(Program());
		if (lexeme != lexemes_list.end()) throw ParserError("Parser: Error : The program does not end after ‘.’;");
	}
	catch (ParserError& pr) {
		errors.push_back(pr.what());
	}
	return;
}

shared_ptr<Parser::Node> Parser::Program() {
	auto this_node = make_shared<Node>("<program>");
	if (GetLexeme()->code != grammar->key_words["PROGRAM"]) ThrowErr("PROGRAM", GetLexeme());
	else this_node->children.emplace_back(make_shared<Node>(lexeme));
	Scan();
	this_node->children.push_back(ProcedureIdentifier());
	if (GetLexeme()->code != ';') ThrowErr(";", GetLexeme());
	else this_node->children.emplace_back(make_shared<Node>(lexeme));
	Scan();
	this_node->children.push_back(Block());
	if (GetLexeme()->code != '.') ThrowErr(".", GetLexeme());
	else this_node->children.emplace_back(make_shared<Node>(lexeme));
	Scan();
	return this_node;
}

shared_ptr<Parser::Node> Parser::ProcedureIdentifier() {
	auto this_node = make_shared<Node>("<procedure-identifier>");
	this_node->children.push_back(Identifier());
	return this_node;
}

shared_ptr<Parser::Node> Parser::ConstantIdentifier() {
	auto this_node = make_shared<Node>("<constant-identifier>");
	this_node->children.push_back(Identifier());
	return this_node;
}

shared_ptr<Parser::Node> Parser::Block() {
	auto this_node = make_shared<Node>("<block>");
	this_node->children.push_back(Declarations());
	if (GetLexeme()->code != grammar->key_words["BEGIN"]) ThrowErr("BEGIN", GetLexeme());
	else this_node->children.emplace_back(make_shared<Node>(lexeme));
	Scan();
	this_node->children.push_back(StatementsList());
	if (GetLexeme()->code != grammar->key_words["END"]) ThrowErr("END", GetLexeme());
	else this_node->children.emplace_back(make_shared<Node>(lexeme));
	Scan();
	return this_node;
}

shared_ptr<Parser::Node> Parser::StatementsList() {
	auto this_node = make_shared<Node>("<statements-list>");
	this_node->children.push_back(Empty());
	return this_node;
}

shared_ptr<Parser::Node> Parser::Declarations() {
	auto this_node = make_shared<Node>("<declarations>");
	this_node->children.push_back(ConstantDeclarations());
	return this_node;
}

shared_ptr<Parser::Node> Parser::ConstantDeclarations() {
	auto this_node = make_shared<Node>("<constant-declarations>");
	if (GetLexeme()->code != grammar->key_words["CONST"]) ThrowErr("CONST", GetLexeme());
	else this_node->children.emplace_back(make_shared<Node>(lexeme));
	Scan();
	if (GetLexeme()->code < 1001)  this_node->children.push_back(Empty());
	else this_node->children.push_back(ConstantDeclarationsList());
	return this_node;
}

shared_ptr<Parser::Node> Parser::ConstantDeclarationsList() {
	auto this_node = make_shared<Node>("<constant-declarations-list>");
	this_node->children.push_back(ConstantDeclaration());
	if (GetLexeme()->code >= 1001) this_node->children.push_back(ConstantDeclarationsList());
	else this_node->children.push_back(Empty());;
	return this_node;
}

shared_ptr<Parser::Node> Parser::ConstantDeclaration() {
	auto this_node = make_shared<Node>("<constant-declaration>");
	this_node->children.push_back(ConstantIdentifier());
	if (GetLexeme()->code != '=') ThrowErr("=", GetLexeme());
	else this_node->children.emplace_back(make_shared<Node>(lexeme));
	Scan();
	this_node->children.push_back(Constant());
	if (GetLexeme()->code != ';') ThrowErr(";", GetLexeme());
	else this_node->children.emplace_back(make_shared<Node>(lexeme));
	Scan();
	return this_node;
}

shared_ptr<Parser::Node> Parser::Constant() {
	auto this_node = make_shared<Node>("<constant>");
	if(GetLexeme()->code < 501 || GetLexeme()->code > 1000) ThrowErr("<complex-constant>", GetLexeme());
	else this_node->children.emplace_back(make_shared<Node>(lexeme));
	Scan();
	return this_node;
}

shared_ptr<Parser::Node> Parser::Empty() {
	auto this_node = make_shared<Node>("<empty>");
	return this_node;
}

shared_ptr<Parser::Node> Parser::Identifier() {
	auto this_node = make_shared<Node>("<identifier>");
	if (GetLexeme()->code < 1001) ThrowErr("<identifier>", GetLexeme());
	else this_node->children.emplace_back(make_shared<Node>(lexeme));
	Scan();
	return this_node;
}