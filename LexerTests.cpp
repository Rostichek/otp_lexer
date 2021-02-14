#include "LexerTests.h"
#include <sstream>
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
	program << "                PROGRAM    \r         \n\nBEGIN   \t   END         ";
	Lexer lexer(CreateGrammar(), program);
	lexer.Parse();
	vector<Lexer::LexemesList::Item> expect_list;
	expect_list.push_back({ 401, 0 });
	expect_list.push_back({ 402, 7 });
	expect_list.push_back({ 403, 12 });
	ASSERT_EQUAL(lexer.GetProgram(), "PROGRAMBEGINEND");
	ASSERT_EQUAL(lexer.GetTokens(), expect_list);
}

void TestComments() {
	{
		stringstream program;
		program << "(**************)PROGRAM(**ssssssssss*ssssss*)\nBEGIN(**)\nEND(*comment\nAcomment*)";
		Lexer lexer(CreateGrammar(), program);
		lexer.Parse();
		vector<Lexer::LexemesList::Item> expect_list;
		expect_list.push_back({ 401, 0 });
		expect_list.push_back({ 402, 7 });
		expect_list.push_back({ 403, 12 });
		ASSERT_EQUAL(lexer.GetProgram(), "PROGRAMBEGINEND");
		ASSERT_EQUAL(lexer.GetTokens(), expect_list);
	}
	{
		stringstream program;
		program << "(**************)PROGRAM(**ssssssssss*ssssss*)\nBEGIN(**)\nEND(*comment\nAcomment";
		Lexer lexer(CreateGrammar(), program);
		lexer.Parse();
		vector<Lexer::LexemesList::Item> expect_list;
		expect_list.push_back({ 401, 0 });
		expect_list.push_back({ 402, 7 });
		expect_list.push_back({ 403, 12 });
		ASSERT_EQUAL(lexer.GetProgram(), "PROGRAMBEGINEND");
		ASSERT_EQUAL(lexer.GetTokens(), expect_list);
		ASSERT_EQUAL(lexer.GetErrors().size(), 1);
	}
}

void TestDelimiters() {
	stringstream program;
	program << ";=";
	Lexer lexer(CreateGrammar(), program);
	lexer.Parse();
	ASSERT(lexer.GetErrors().empty());
	vector<Lexer::LexemesList::Item> expect_list;
	expect_list.push_back({ ';', 0 });
	expect_list.push_back({ '=', 1 });
	ASSERT_EQUAL(lexer.GetTokens(), expect_list);
	ASSERT_EQUAL(lexer.GetProgram(), ";=");
}

void TestKeywordsOrIdentifiers() {
	stringstream program;
	program << "PROGRAM        BEGIN   END    CONST PROGRAM1    A ENB  A   ";
	auto grammar = CreateGrammar();
	Lexer lexer(grammar, program);
	lexer.Parse();
	ASSERT(lexer.GetErrors().empty());
	vector<Lexer::LexemesList::Item> expect_list;
	expect_list.push_back({ 401, 0 });
	expect_list.push_back({ 402, 7 });
	expect_list.push_back({ 403, 12 });
	expect_list.push_back({ 404, 15 });
	expect_list.push_back({ 1001, 20 });
	expect_list.push_back({ 1002, 28 });
	expect_list.push_back({ 1003, 29 });
	expect_list.push_back({ 1002, 32 });

	unordered_map<string, Code> expect_indentifiers =
	{
		{"PROGRAM1", 1001},
		{"A", 1002},
		{"ENB", 1003}
	};
	ASSERT_EQUAL(grammar->identifiers, expect_indentifiers);
	ASSERT_EQUAL(lexer.GetTokens(), expect_list);
	ASSERT_EQUAL(lexer.GetProgram(), "PROGRAMBEGINENDCONSTPROGRAM1AENBA");
}

void TestConstants() {
	{
		stringstream program;
		program << R"('   10    20  '   '   10    $EXP(  20 )  ')";
		auto grammar = CreateGrammar();
		Lexer lexer(grammar, program);
		lexer.Parse();
		ASSERT(lexer.GetErrors().empty());
		vector<Lexer::LexemesList::Item> expect_list;
		expect_list.push_back({ 501, 0 });
		expect_list.push_back({ 502, 6 });
		unordered_map<string, Code> expect_constants =
		{
			{R"(1020)", 501},
			{"10$EXP(20)", 502},
		};
		ASSERT_EQUAL(grammar->constants, expect_constants);
		ASSERT_EQUAL(lexer.GetTokens(), expect_list);
		ASSERT_EQUAL(lexer.GetProgram(), R"('1020''10$EXP(20)')");
	}
	{
		stringstream program;
		program << R"(' 20 ' '10 '  '' '10 ')";
		auto grammar = CreateGrammar();
		Lexer lexer(grammar, program);
		lexer.Parse();
		ASSERT(lexer.GetErrors().empty());
		vector<Lexer::LexemesList::Item> expect_list;
		expect_list.push_back({ 501, 0 });
		expect_list.push_back({ 502, 4 });
		expect_list.push_back({ 503, 8 });
		expect_list.push_back({ 502, 10 });
		unordered_map<string, Code> expect_constants =
		{
			{R"(20)", 501},
			{R"(10)", 502},
			{R"()", 503}
		};
		ASSERT_EQUAL(grammar->constants, expect_constants);
		ASSERT_EQUAL(lexer.GetTokens(), expect_list);
		ASSERT_EQUAL(lexer.GetProgram(), R"('20''10''''10')");
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
	vector<Lexer::LexemesList::Item> expect_list;
	expect_list.push_back({ 401, 0 });
	expect_list.push_back({ 1001, 7 });
	expect_list.push_back({ 59, 12 });
	expect_list.push_back({ 404, 13 });
	expect_list.push_back({ 1002, 18 });
	expect_list.push_back({ 61, 22 });
	expect_list.push_back({ 501, 23 });
	expect_list.push_back({ 1003, 28 });
	expect_list.push_back({ 61, 32 });
	expect_list.push_back({ 502, 33 });
	expect_list.push_back({ 1004, 36 });
	expect_list.push_back({ 61, 41 });
	expect_list.push_back({ 503, 42 });
	expect_list.push_back({ 1005, 44 });
	expect_list.push_back({ 61, 52 });
	expect_list.push_back({ 504, 53 });
	expect_list.push_back({ 1006, 59 });
	expect_list.push_back({ 61, 67 });
	expect_list.push_back({ 505, 68 });
	expect_list.push_back({ 402, 79 });
	expect_list.push_back({ 403, 84 });
	ASSERT_EQUAL(lexer.GetTokens(), expect_list);
	ASSERT_EQUAL(lexer.GetProgram(), R"(PROGRAMTEST1;CONSTVAL1='100'VAL2='4'EMPTY=''COMPLEX1='1010'COMPLEX2='10$EXP(3)'BEGINEND)");
	unordered_map<string, Code> expect_constants =
	{
		{R"(100)", 501},
		{R"(4)", 502},
		{R"()", 503},
		{R"(1010)", 504},
		{R"(10$EXP(3))", 505},
	};
	ASSERT_EQUAL(grammar->constants, expect_constants);
	unordered_map<string, Code> expect_indentifiers =
	{
		{"TEST1", 1001},
		{"VAL1", 1002},
		{"VAL2", 1003},
		{"EMPTY", 1004},
		{"COMPLEX1", 1005},
		{"COMPLEX2", 1006},
	};
	ASSERT_EQUAL(grammar->identifiers, expect_indentifiers);
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
