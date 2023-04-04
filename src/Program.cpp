#include "Program.hpp"

#include <iostream>

Program::Program(FuncDefs_t &&funcs)
	: m_Funcs(std::move(funcs))
{}

void Program::load(std::function<void(const FuncDefs_t &)> onLoad) const
{
	onLoad(m_Funcs);
}