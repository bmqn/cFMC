#pragma once

#include <unordered_map>
#include <vector>
#include <stack>
#include <utility>

#include "Term.hpp"
#include "Parser.hpp"

using BoundVars_t = std::unordered_map<Var, const Term *, VarHash>;

using BindCtx_t   = std::unordered_map<const Term *, BoundVars_t>;
using Frame_t     = std::stack<std::pair<const Term *, BoundVars_t>>;

using Stacks_t    = std::unordered_map<Loc, std::vector<const Term *>, LocHash>;

class Machine
{
public:
	Machine() = delete;
	Machine(const Machine &machine) = delete;
	Machine(Machine &&machine) = delete;

	Machine(const FuncDefs_t *funcs);

	void execute();
	void printDebug();

private:
	void execute(const Term &term);

private:
	const FuncDefs_t *m_Funcs;

	// Look into closures !
	BindCtx_t m_BindCtx;
	Frame_t m_Frame;

	Stacks_t m_Stacks;
};