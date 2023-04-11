#include "Program.hpp"

Program::Program(FuncDefs_t &&funcs)
	: m_Funcs(std::move(funcs))
{}

std::optional<TermHandle_t> Program::load(const std::string &funcName) const
{
	auto it = m_Funcs.find(funcName);
	if (it != m_Funcs.end())
	{
		return it->second;
	}
	return std::nullopt;
}