#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <functional>

#include "Term.hpp"

using FuncDefs_t = std::unordered_map<std::string, Term>;

class Program
{
public:
	Program() = delete;
	Program(const Program &program) = delete;
	Program(Program &&program) = delete;

	Program(FuncDefs_t &&funcs);

	void load(std::function<void(const FuncDefs_t *)>) const;

private:
	std::shared_ptr<FuncDefs_t> m_Funcs;
};