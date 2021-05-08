#pragma once
#include "Lexer.h"
#include "test_runner.h"
#include <iomanip>
#include <memory>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ostream>
#include <string>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include "Lexer.h"
#include "Parser.h"
#include "LexerTests.h"
#include "test_runner.h"
#include <unordered_set>
#include "Generator.h"

std::shared_ptr<Parse::Grammar> CreateGrammar();

void RunTests(const std::string& path);
void StartTest(const std::string& path);
void CheckTests(const std::string& path);


