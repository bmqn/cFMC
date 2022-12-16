#pragma once

#include <memory>
#include <variant>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <cstdint>

#include "Lexer.hpp"

using Loc_t = std::string;
using Var_t = std::string;

const Loc_t k_DefaultLoc = "dfl";
const Loc_t k_OutputLoc = "out";
const Loc_t k_RandomLoc = "rnd";

struct Loc;
struct Var;

class Term;
struct NilTerm;
struct VarContTerm;
struct AbsTerm;
struct AppTerm;

struct Loc
{
	Loc() : loc() {}
	Loc(std::string_view l) : loc(l) {}
	Loc(Loc_t l) : loc(l) {}

	friend bool operator==(const Loc &lhs, const Loc &rhs)
	{
		return lhs.loc == rhs.loc;
	}

	Loc_t loc;
};

struct LocHash
{
	std::size_t operator()(const Loc &loc) const
	{
		return std::hash<std::string>()(loc.loc);
	}
};

struct Var
{
	Var() : var() {}
	Var(Var_t v) : var(v) {}

	friend bool operator==(const Var &lhs, const Var &rhs)
	{
		return lhs.var == rhs.var;
	}

	Var_t var;
};

struct VarHash
{
	std::size_t operator()(const Var &var) const
	{
		return std::hash<std::string>()(var.var);
	}
};

// N  ::=  *  |  x.N  |  [M]a.N  |  a<x>.N

struct NilTerm
{
};

struct VarContTerm
{
	VarContTerm();
	VarContTerm(Var var);

	Var var;
	Term *body;
};

struct AbsTerm
{
	AbsTerm();

	Loc loc;
	Var var;
	Term *body;
};

struct AppTerm
{
	AppTerm();

	Loc loc;
	Term *arg;
	Term *body;
};

class Term
{
private:
	using Term_t = std::variant<
		NilTerm, VarContTerm, AbsTerm, AppTerm
	>;

public:
	enum Kind
	{
		Nil, VarCont, Abs, App,
	};

	Term() = delete;

	template<typename T>
	Term(Kind kind, T term)
		: m_Kind(kind), m_Term(term)
	{}

	Kind kind() const
	{
		return m_Kind;
	}

	template<typename T>
	T term() const
	{
		return std::get<T>(m_Term);
	}

private:
	Kind m_Kind;
	Term_t m_Term;
};

class Program
{
public:
	Program()
		: m_Entry(nullptr)
	{
	}

	void setEntry(const Term term)
	{
		m_Entry = std::make_unique<Term>(term);
	}

	const Term *getEntry() const
	{
		return m_Entry.get();
	}

private:
	std::unique_ptr<Term> m_Entry;
};

class Parser
{
public:

	Parser();

	Program parse(const std::string &programSrc);

private:
	std::optional<Term> parseMain();

	std::optional<Term> parseTerm();

	std::optional<AbsTerm> parseAbstraction(Loc loc = Loc(k_DefaultLoc));
	std::optional<AppTerm> parseApplication(Loc loc = Loc(k_DefaultLoc));

private:
	std::unique_ptr<Lexer> m_Lexer;
};