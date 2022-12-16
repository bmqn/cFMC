#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <unordered_map>

#include "Parser.hpp"

using BoundVars_t = std::unordered_map<Var, const Term *, VarHash>;
using Stack_t     = std::vector<const Term *>;
using Scope_t     = std::pair<const Term *, BoundVars_t>;

Scope_t createScope();

class Machine
{
public:
	Machine();

	void execute(const Program &program);
	void printDebug();

private:
	std::unordered_map<Loc, Stack_t, LocHash> m_Stacks;
	std::stack<Scope_t> m_Scope;
};