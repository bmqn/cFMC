#pragma once

#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <stack>
#include <unordered_map>

#include "Parser.hpp"

using BoundVars_t = std::unordered_map<Var, const Term *, VarHash>;
using Stack_t     = std::vector<const Term *>;
using Env_t       = std::pair<const Term *, BoundVars_t>;

Env_t createScope();

class Machine
{
public:
	Machine();

	void execute(Program &program);
	void printDebug();

private:
	void execute(const Term &term);

private:
	std::unordered_map<Loc, Stack_t, LocHash> m_Stacks;
};