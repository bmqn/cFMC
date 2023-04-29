#pragma once

#include <unordered_map>
#include <vector>
#include <utility>

#include "Term.hpp"
#include "Parser.hpp"

// Ouch.. using a void pointer here is rough :/
using VarEnv_t = std::unordered_map<Var_t, std::shared_ptr<void>>;
using LocVarEnv_t = std::unordered_map<LocVar_t, Loc_t>;
using Env_t = std::pair<VarEnv_t, LocVarEnv_t>;

using Closure_t = std::pair<Env_t, TermHandle_t>;
using ClosureStack_t = std::vector<Closure_t>;
using ClosureMemory_t = std::unordered_map<Loc_t, ClosureStack_t>;

using Callstack_t = std::vector<std::pair<std::string, TermHandle_t>>;

class Machine
{
public:
	void execute(const Program &funcs);

	std::string getStackDebug() const;
	std::string getCallstackDebug() const;

private:
	std::optional<Closure_t> tryPop(Env_t env, Loc_t loc);
	std::optional<Prim_t> tryPopPrim(Env_t env, Loc_t loc);
	std::optional<Loc_t> tryPopLoc(Env_t env, Loc_t loc);

	TermHandle_t freshTerm(Term &&term);

private:
	ClosureMemory_t m_Memory;
	ClosureStack_t m_Control;

	Callstack_t m_CallStack;

	std::vector<TermOwner_t> m_FreshTerms;
};