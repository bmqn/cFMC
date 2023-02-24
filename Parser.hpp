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
	Term parseTerm(const std::string &termSrc);

private:
	FuncDefs_t parseFuncDefs();

	std::optional<Term> parseTerm();
	std::optional<AbsTerm> parseAbstraction(Loc_t loc = k_DefaultLoc);
	std::optional<AppTerm> parseApplication(Loc_t loc = k_DefaultLoc);

private:
	std::unique_ptr<Lexer> m_Lexer;
};