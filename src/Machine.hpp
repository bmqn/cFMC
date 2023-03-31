#pragma once

#include <unordered_map>
#include <vector>
#include <utility>

#include "Term.hpp"
#include "Parser.hpp"

using Stacks_t = std::unordered_map<Loc_t, std::vector<const Term *>>;

using BoundVars_t = std::unordered_map<Var_t, const Term *>;
using BoundLocVars_t = std::unordered_map<LocVar_t, Loc_t>;

template<typename BoundT>
using BindCtx_t = std::unordered_map<const Term *, BoundT>;

using Frame_t = std::vector<std::tuple<const Term *, std::pair<BoundVars_t, BoundLocVars_t>>>;

class Machine
{
public:
	Machine() = delete;
	Machine(const Machine &machine) = delete;
	Machine(Machine &&machine) = delete;

	Machine(const Program::FuncDefs_t *funcs);

	void execute();
	
	std::string getStackDebug() const;
	std::string getCallstackDebug() const;

private:
	void execute(const Term &term);

private:
	const Program::FuncDefs_t *m_Funcs;
	
	Stacks_t m_Stacks;
	BindCtx_t<BoundVars_t> m_VarBindCtx;
	BindCtx_t<BoundLocVars_t> m_LocVarBindCtx;
	Frame_t m_Frame;

	std::vector<std::pair<std::string, const Term *>> m_CallStack;
};