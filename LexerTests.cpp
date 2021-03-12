#include "LexerTests.h"
#include <sstream>
#include <fstream>
#include "profile.h"

using namespace Parse;
using namespace std;

shared_ptr<Grammar> CreateGrammar() {
	auto gramar = make_shared<Grammar>();
	gramar->symbols_attributes.fill(10);
	for (size_t i = 0; i < gramar->symbols_attributes.size(); ++i) {
		if ((i >= 'A' && i <= 'Z') || (i >= 'a' && i <= 'z'))
			gramar->symbols_attributes[i] = 2;
		else if (i >= '0' && i <= '9')
			gramar->symbols_attributes[i] = 1;
		if (i >= 9 && i <= 13)
			gramar->symbols_attributes[i] = 0;
	}
	gramar->symbols_attributes[40] = 4;
	gramar->symbols_attributes[32] = 0;
	gramar->symbols_attributes[36] = 5;
	gramar->symbols_attributes[39] = 6;
	gramar->symbols_attributes[59] = 3;
	gramar->symbols_attributes[61] = 3;
	gramar->key_words["PROGRAM"] = 401;
	gramar->key_words["BEGIN"] = 402;
	gramar->key_words["END"] = 403;
	gramar->key_words["CONST"] = 404;
	return gramar;
}

void TestWhitespaces() {
	stringstream program;
	program << "                PROGRAM             \n\nBEGIN   \t   END         ";
	Lexer lexer(CreateGrammar(), program);
	lexer.Parse();
	vector<Lexer::LexemesList::Item> expected_list;
	expected_list.push_back({ 401, Position{1,17} });
	expected_list.push_back({ 402, Position{3,1} });
	expected_list.push_back({ 403, Position{3,16} });
	ASSERT_EQUAL(lexer.GetTokens(), expected_list);
}

void TestComments() {
	{
		stringstream program;
		program << "(**************)PROGRAM(**ssssss__ssss*ssssss*)\nBEGIN(**)\nEND(*comment\nAcomment*)";
		Lexer lexer(CreateGrammar(), program);
		lexer.Parse();
		vector<Lexer::LexemesList::Item> expected_list;
		expected_list.push_back({ 401, Position{1,17} });
		expected_list.push_back({ 402, Position{2,1} });
		expected_list.push_back({ 403, Position{3,1} });
		ASSERT_EQUAL(lexer.GetTokens(), expected_list);
	}
	{
		stringstream program;
		program << "(**************)PROGRAM(**ssssssssss*ssssss*)\nBEGIN(**)\nEND(*comment\nAcomment";
		Lexer lexer(CreateGrammar(), program);
		lexer.Parse();
		vector<Lexer::LexemesList::Item> expected_list;
		expected_list.push_back({ 401, Position{1,17} });
		expected_list.push_back({ 402, Position{2,1} });
		expected_list.push_back({ 403, Position{3,1} });
		ASSERT_EQUAL(lexer.GetTokens(), expected_list);
		ASSERT_EQUAL(lexer.GetErrors().size(), 1);
	}
}

void TestDelimiters() {
	stringstream program;
	program << ";==";
	Lexer lexer(CreateGrammar(), program);
	lexer.Parse();
	ASSERT(lexer.GetErrors().empty());
	vector<Lexer::LexemesList::Item> expected_list;
	expected_list.push_back({ ';', Position{1,1} });
	expected_list.push_back({ '=', Position{1,2} });
	expected_list.push_back({ '=', Position{1,3} });
	ASSERT_EQUAL(lexer.GetTokens(), expected_list);
}

void TestKeywordsOrIdentifiers() {
	stringstream program;
	program << "PROGRAM        BEGIN   END    CONST PROGRAM1    A ENB  A   ";
	auto grammar = CreateGrammar();
	Lexer lexer(grammar, program);
	lexer.Parse();
	ASSERT(lexer.GetErrors().empty());
	vector<Lexer::LexemesList::Item> expected_list;
	expected_list.push_back({ 401, Position{1,1} });
	expected_list.push_back({ 402, Position{1,16} });
	expected_list.push_back({ 403, Position{1,24} });
	expected_list.push_back({ 404, Position{1,31} });
	expected_list.push_back({ 1001, Position{1,37} });
	expected_list.push_back({ 1002, Position{1,49} });
	expected_list.push_back({ 1003, Position{1,51} });
	expected_list.push_back({ 1002, Position{1,56} });

	unordered_map<string, Code> expected_indentifiers =
	{
		{"PROGRAM1", 1001},
		{"A", 1002},
		{"ENB", 1003}
	};
	ASSERT_EQUAL(grammar->identifiers, expected_indentifiers);
	ASSERT_EQUAL(lexer.GetTokens(), expected_list);
}

void TestConstants() {
	{
		stringstream program;
		program << R"('   10    20  '   '   10    $EXP(  20 )  ')";
		auto grammar = CreateGrammar();
		Lexer lexer(grammar, program);
		lexer.Parse();
		ASSERT(lexer.GetErrors().empty());
		vector<Lexer::LexemesList::Item> expected_list;
		expected_list.push_back({ 501, Position{1,1} });
		expected_list.push_back({ 502, Position{1,19} });
		unordered_map<string, Code> expected_constants =
		{
			{R"('10 20')", 501},
			{"'10 $EXP(20)'", 502},
		};
		ASSERT_EQUAL(grammar->constants, expected_constants);
		ASSERT_EQUAL(lexer.GetTokens(), expected_list);
	}
	{
		stringstream program;
		program << R"(' 20 ' '10 '  '' '10 ')";
		auto grammar = CreateGrammar();
		Lexer lexer(grammar, program);
		lexer.Parse();
		ASSERT(lexer.GetErrors().empty());
		vector<Lexer::LexemesList::Item> expected_list;
		expected_list.push_back({ 501, Position{1,1} });
		expected_list.push_back({ 502, Position{1,8} });
		expected_list.push_back({ 503, Position{1,15} });
		expected_list.push_back({ 502, Position{1,18} });
		unordered_map<string, Code> expected_constants =
		{
			{R"('20')", 501},
			{R"('10')", 502},
			{R"('')", 503}
		};
		ASSERT_EQUAL(grammar->constants, expected_constants);
		ASSERT_EQUAL(lexer.GetTokens(), expected_list);
	}
}

void TestProgram() {
	stringstream program;
	program << R"(
		PROGRAM TEST1; 
		CONST
			VAL1 = '100'	
			VAL2 = '4'
			EMPTY = ''
			COMPLEX1 = '10 10'
			COMPLEX2 = '10 $EXP(3)'
		BEGIN
				
		END
	)";
	auto grammar = CreateGrammar();
	Lexer lexer(grammar, program);
	lexer.Parse();
	ASSERT(lexer.GetErrors().empty());
	vector<Lexer::LexemesList::Item> expected_list;
	expected_list.push_back({ 401, Position{2,9} });
	expected_list.push_back({ 1001, Position{2,17} });
	expected_list.push_back({ 59, Position{2,22} });
	expected_list.push_back({ 404, Position{3,9} });
	expected_list.push_back({ 1002, Position{4,13} });
	expected_list.push_back({ 61, Position{4,18} });
	expected_list.push_back({ 501, Position{4,20} });
	expected_list.push_back({ 1003, Position{5,13} });
	expected_list.push_back({ 61, Position{5,18} });
	expected_list.push_back({ 502, Position{5,20} });
	expected_list.push_back({ 1004, Position{6,13} });
	expected_list.push_back({ 61, Position{6,19} });
	expected_list.push_back({ 503, Position{6,21} });
	expected_list.push_back({ 1005, Position{7,13} });
	expected_list.push_back({ 61, Position{7,22} });
	expected_list.push_back({ 504, Position{7,24} });
	expected_list.push_back({ 1006, Position{8,13} });
	expected_list.push_back({ 61, Position{8,22} });
	expected_list.push_back({ 505, Position{8,24} });
	expected_list.push_back({ 402, Position{9,9} });
	expected_list.push_back({ 403, Position{11,9} });
	ASSERT_EQUAL(lexer.GetTokens(), expected_list);
	unordered_map<string, Code> expected_constants =
	{
		{R"('100')", 501},
		{R"('4')", 502},
		{R"('')", 503},
		{R"('10 10')", 504},
		{R"('10 $EXP(3)')", 505},
	};
	ASSERT_EQUAL(grammar->constants, expected_constants);
	unordered_map<string, Code> expected_indentifiers =
	{
		{"TEST1", 1001},
		{"VAL1", 1002},
		{"VAL2", 1003},
		{"EMPTY", 1004},
		{"COMPLEX1", 1005},
		{"COMPLEX2", 1006},
	};
	ASSERT_EQUAL(grammar->identifiers, expected_indentifiers);
}

void TestErrors() {
	stringstream program;
	program << R"(
		PROGRAM TEST1; 
		CONST
			VAL_1 = '100'	
			VAL2 = '4'
			1EMPTY = ''
			COMPLEX1 = '10 &EXP(4)'
			COMPLEX2 = '10 10'
			COMPLEX3 = '10 $EXP(3'
		BEGIN
			(*	
		END
	)";
	Lexer lexer(CreateGrammar(), program);
	lexer.Parse();
	ASSERT_EQUAL(lexer.GetErrors().size(), 6);
}

void RunLexerTests(TestRunner& tr) {
	RUN_TEST(tr, TestWhitespaces);
	RUN_TEST(tr, TestComments);
	RUN_TEST(tr, TestDelimiters);
	RUN_TEST(tr, TestKeywordsOrIdentifiers);
	RUN_TEST(tr, TestConstants);
	RUN_TEST(tr, TestProgram);
	RUN_TEST(tr, TestErrors);
}
