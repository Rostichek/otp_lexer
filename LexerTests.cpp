#include "LexerTests.h"

using namespace Parse;
using namespace std;

shared_ptr<Gramar> CreateGramar() {
	auto gramar = make_shared<Gramar>();
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
	Lexer lexer(CreateGramar());
	string program = "                PROGRAM    \r         \n\nBEGIN   \t   END         ";
	ASSERT_EQUAL(lexer.Parse(program).program, "PROGRAMBEGINEND");
}

void TestComments() {
	Lexer lexer(CreateGramar());
	string program = "(**************)PROGRAM(**ssssssssss*ssssss*)\nBEGIN(**)\nEND(*comment\nAcomment*)";
	ASSERT_EQUAL(lexer.Parse(program).program, "PROGRAMBEGINEND");

	program = "(**************)PROGRAM(**ssssssssss*ssssss*)\nBEGIN(**)\nEND(*comment\nAcomment";
	auto lexeme_list = lexer.Parse(program);
	ASSERT_EQUAL(lexeme_list.program, "PROGRAMBEGINEND");
	ASSERT_EQUAL(lexeme_list.errors.size(), 1);
}

void TestDelimiters() {
	Lexer lexer(CreateGramar());
	string program = ";=";
	auto lexeme_list = lexer.Parse(program);
	ASSERT(lexeme_list.errors.empty());
	Lexer::LexemesList expect_list;
	expect_list.items.push_back({ ';', 0 });
	expect_list.items.push_back({ '=', 1 });
	expect_list.program = ";=";
	ASSERT_EQUAL(lexeme_list, expect_list);
}

void TestKeywordsOrIdentifiers() {
	Lexer lexer(CreateGramar());
	string program = "PROGRAM        BEGIN   END    CONST PROGRAM1    A ENB     ";
	auto lexeme_list = lexer.Parse(program);
	ASSERT(lexeme_list.errors.empty());
	Lexer::LexemesList expect_list;
	expect_list.items.push_back({ 401, 0 });
	expect_list.items.push_back({ 402, 7 });
	expect_list.items.push_back({ 403, 12 });
	expect_list.items.push_back({ 404, 15 });
	expect_list.items.push_back({ 1001, 20 });
	expect_list.items.push_back({ 1002, 28 });
	expect_list.items.push_back({ 1003, 29 });

	expect_list.program = "PROGRAMBEGINENDCONSTPROGRAM1AENB";
	ASSERT_EQUAL(lexeme_list, expect_list);
}

void TestConstants() {
	{
		Lexer lexer(CreateGramar());
		string program = R"('   10    20  '   '   10    $EXP(  20 )  ')";
		auto lexeme_list = lexer.Parse(program);
		ASSERT(lexeme_list.errors.empty());
		Lexer::LexemesList expect_list;
		expect_list.items.push_back({ 501, 0 });
		expect_list.items.push_back({ 502, 6 });
		expect_list.program = R"('1020''10$EXP(20)')";
		ASSERT_EQUAL(lexeme_list, expect_list);
	}
	{
		Lexer lexer(CreateGramar());
		string program = R"(' 20 ' '10 '  '')";
		auto lexeme_list = lexer.Parse(program);
		ASSERT(lexeme_list.errors.empty());
		Lexer::LexemesList expect_list;
		expect_list.items.push_back({ 501, 0 });
		expect_list.items.push_back({ 502, 4 });
		expect_list.items.push_back({ 503, 8 });
		expect_list.program = R"('20''10''')";
		ASSERT_EQUAL(lexeme_list, expect_list);
	}
}

void TestProgram() {
	Lexer lexer(CreateGramar());
	string program = R"(
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
	auto lexeme_list = lexer.Parse(program);
	ASSERT(lexeme_list.errors.empty());
	Lexer::LexemesList expect_list;
	expect_list.program = R"(PROGRAMTEST1;CONSTVAL1='100'VAL2='4'EMPTY=''COMPLEX1='1010'COMPLEX2='10$EXP(3)'BEGINEND)";
	expect_list.items.push_back({ 401, 0 });
	expect_list.items.push_back({ 1001, 7 });
	expect_list.items.push_back({ 59, 12 });
	expect_list.items.push_back({ 404, 13 });
	expect_list.items.push_back({ 1002, 18 });
	expect_list.items.push_back({ 61, 22 });
	expect_list.items.push_back({ 501, 23 });
	expect_list.items.push_back({ 1003, 28 });
	expect_list.items.push_back({ 61, 32 });
	expect_list.items.push_back({ 502, 33 });
	expect_list.items.push_back({ 1004, 36 });
	expect_list.items.push_back({ 61, 41 });
	expect_list.items.push_back({ 503, 42 });
	expect_list.items.push_back({ 1005, 44 });
	expect_list.items.push_back({ 61, 52 });
	expect_list.items.push_back({ 504, 53 });
	expect_list.items.push_back({ 1006, 59 });
	expect_list.items.push_back({ 61, 67 });
	expect_list.items.push_back({ 505, 68 });
	expect_list.items.push_back({ 402, 79 });
	expect_list.items.push_back({ 403, 84 });
	ASSERT_EQUAL(lexeme_list, expect_list);
}

void TestErrors() {
	Lexer lexer(CreateGramar());
	string program = R"(
		PROGRAM TEST1; 
		CONST
			VAL_1 = '100'	
			VAL2 = '4'
			1EMPTY = ''
			COMPLEX1 = '10 10'
			COMPLEX2 = '10 $EXP(3'
		BEGIN
			(*	
		END
	)";
	ASSERT_EQUAL(lexer.Parse(program).errors.size(), 5);
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
