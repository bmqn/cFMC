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

	std::optional<AbsTerm> parseAbstraction(Var_t loc = std::string(k_DefaultLocId));
	std::optional<AppTerm> parseApplication(Var_t loc = std::string(k_DefaultLocId));

	std::optional<std::pair<Val_t, Term>> parseCase();
	std::optional<CasesTerm> parseCases();

private:
	std::unique_ptr<Lexer> m_Lexer;
};