#pragma once

#include <unordered_map>
#include <vector>
#include <stack>
#include <utility>

#include "Term.hpp"
#include "Parser.hpp"

using Stacks_t = std::unordered_map<Loc_t, std::vector<const Term *>>;

using BoundVars_t = std::unordered_map<Var_t, const Term *>;
using BoundLocVars_t = std::unordered_map<LocVar_t, Loc_t>;

template<typename BoundT>
using BindCtx_t = std::unordered_map<const Term *, BoundT>;

using Frame_t = std::stack<std::tuple<const Term *, std::pair<BoundVars_t, BoundLocVars_t>>>;

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
	
	Stacks_t m_Stacks;

	// TODO: Look into closures !
	BindCtx_t<BoundVars_t> m_VarBindCtx;
	BindCtx_t<BoundLocVars_t> m_LocVarBindCtx;
	Frame_t m_Frame;
};