#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <functional>

#include "Term.hpp"

class Program
{
public:
	using FuncDefs_t = std::unordered_map<std::string, Term>;

public:
	Program() = delete;
	Program(const Program &program) = delete;
	Program(Program &&program) = delete;

	Program(FuncDefs_t &&funcs);

	void load(std::function<void(const FuncDefs_t *)>) const;

private:
	std::shared_ptr<FuncDefs_t> m_Funcs;
};