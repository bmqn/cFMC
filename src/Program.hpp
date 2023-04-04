#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <functional>

#include "Term.hpp"

class Program
{
public:
	using FuncDefs_t = std::unordered_map<std::string, TermOwner_t>;

public:
	Program() = delete;
	Program(const Program &program) = delete;
	Program(Program &&program) = delete;

	Program(FuncDefs_t &&funcs);

	void load(std::function<void(const FuncDefs_t &)> onLoad) const;

private:
	FuncDefs_t m_Funcs;
};