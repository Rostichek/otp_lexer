#pragma once
#include "Lexer.h"
#include "test_runner.h"
#include <sstream>
#include <iomanip>
#include <string>
#include <memory>

std::shared_ptr<Parse::Gramar> CreateGramar();

void TestWhitespaces();

void TestComments();

void TestDelimiters();

void TestKeywordsOrIdentifiers();

void TestConstants();

void TestProgram();

void TestErrors();

void RunLexerTests(TestRunner& tr);
