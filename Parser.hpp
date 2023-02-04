#pragma once

#include <memory>
#include <optional>
#include <string>
#include <stack>

#include "Config.hpp"
#include "Lexer.hpp"
#include "Term.hpp"
#include "Program.hpp"

class Parser
{
public:
	Parser();

	Program parseProgram(const std::string &programSrc);
	Term parseTerm(const std::string &programSrc);

private:
	FuncDefs_t parseFuncDefs();

	std::optional<Term> parseTerm(bool handleErrors = true);
	std::optional<AbsTerm> parseAbstraction(Loc loc = k_DefaultLoc);
	std::optional<AppTerm> parseApplication(Loc loc = k_DefaultLoc);

private:
	std::unique_ptr<Lexer> m_Lexer;
};