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
	std::optional<Term> parseTerm(const std::string &termSrc);

private:
	Program::FuncDefs_t parseFuncDefs();

	std::optional<Term> parseTerm();

	std::optional<VarTerm> parseVar();
	std::optional<AbsTerm> parseAbs();
	std::optional<AppTerm> parseApp();
	std::optional<LocAbsTerm> parseLocAbs();
	std::optional<LocAppTerm> parseLocApp();

	std::optional<ValTerm> parseVal();
	std::optional<BinOpTerm> parseBinOp();
	std::optional<CasesTerm<Prim_t>> parsePrimCases();
	std::optional<CasesTerm<Loc_t>> parseLocCases();

private:
	std::unique_ptr<Lexer> m_Lexer;
};