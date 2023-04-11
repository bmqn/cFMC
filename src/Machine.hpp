#pragma once

#include <unordered_map>
#include <vector>
#include <utility>

#include "Term.hpp"
#include "Parser.hpp"

using BoundVars_t = std::unordered_map<Var_t, TermHandle_t>;
using BoundLocVars_t = std::unordered_map<LocVar_t, Loc_t>;

using VarBindCtx_t = std::unordered_map<TermHandle_t, BoundVars_t>;
using LocBindCtx_t = std::unordered_map<TermHandle_t, BoundLocVars_t>;

using Memory_t = std::unordered_map<Loc_t, std::vector<TermHandle_t>>;
using Frame_t = std::vector<std::tuple<TermHandle_t, std::pair<BoundVars_t, BoundLocVars_t>>>;
using Callstack_t = std::vector<std::pair<std::string, TermHandle_t>>;

class Machine
{
public:
	void execute(const Program &funcs);

	std::string getStackDebug() const;
	std::string getCallstackDebug() const;
	std::string getBindDebug() const;

private:
	TermHandle_t freshTerm(Term &&term);

private:
	Memory_t m_Stacks;
	Frame_t m_Frame;
	Callstack_t m_CallStack;

	VarBindCtx_t m_VarBindCtx;
	LocBindCtx_t m_LocVarBindCtx;

	std::vector<TermOwner_t> m_FreshTerms;
};