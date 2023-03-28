#pragma once

#include <memory>
#include <optional>
#include <string>
#include <stack>
#include <variant>

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

	std::optional<VarContTerm> parseVarCont();
	std::optional<std::variant<AbsTerm, LocAbsTerm>> parseAbstraction();
	std::optional<std::variant<AppTerm, LocAppTerm>> parseApplication();

	std::optional<ValTerm> parseVal();

	std::optional<std::pair<std::string, Term>> parseCase();
	std::optional<CasesTerm> parseCases();

private:
	std::unique_ptr<Lexer> m_Lexer;
};