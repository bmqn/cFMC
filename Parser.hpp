#pragma once

#include <memory>
#include <variant>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <cstdint>
#include <stack>

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
	Loc(std::string l) : loc(l) {}
	Loc(std::string_view l) : loc(l) {}

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
	Var(std::string v) : var(v) {}
	Var(std::string_view v) : var(v) {}

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

	VarContTerm(const AbsTerm &other) = delete;
	VarContTerm &operator=(const AbsTerm &other) = delete;

	Var var;
	std::unique_ptr<Term> body;
};

struct AbsTerm
{
	AbsTerm();
	AbsTerm(Loc loc, Var var);

	AbsTerm(const AbsTerm &other) = delete;
	AbsTerm &operator=(const AbsTerm &other) = delete;

	AbsTerm(AbsTerm &&other);
	AbsTerm &operator=(AbsTerm &&other);

	Loc loc;
	Var var;
	std::unique_ptr<Term> body;
};

struct AppTerm
{
	AppTerm();
	AppTerm(Loc loc);

	AppTerm(const AppTerm &other) = delete;
	AppTerm &operator=(const AppTerm &other) = delete;

	AppTerm(AppTerm &&other);
	AppTerm &operator=(AppTerm &&other);

	Loc loc;
	std::unique_ptr<Term> arg;
	std::unique_ptr<Term> body;
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
		Nil, VarCont, Abs, App
	};

	Term() = delete;
	Term(const Term &other) = delete;
	Term &operator=(const Term &other) = delete;

	Term(NilTerm &&term) : m_Kind(Nil), m_Term(std::move(term)) {}
	Term(VarContTerm &&term) : m_Kind(VarCont), m_Term(std::move(term)) {}
	Term(AbsTerm &&term) : m_Kind(Abs), m_Term(std::move(term)) {}
	Term(AppTerm &&term) : m_Kind(App), m_Term(std::move(term)) {}

	Term(Term &&other)
		: m_Kind(std::move(other.m_Kind))
		, m_Term(std::move(other.m_Term))
	{
		other.m_Kind = Nil;
		other.m_Term = NilTerm();
	}

	Term &operator=(Term &&other)
	{
		m_Kind = other.m_Kind;
		m_Term = std::move(other.m_Term);

		other.m_Kind = Nil;
		other.m_Term = NilTerm();

		return *this;
	}

	Kind kind() const
	{
		return m_Kind;
	}

	const NilTerm &asNil() const
	{
		return std::get<NilTerm>(m_Term);
	}

	const VarContTerm &asVarCont() const
	{
		return std::get<VarContTerm>(m_Term);
	}

	const AbsTerm &asAbs() const
	{
		return std::get<AbsTerm>(m_Term);
	}

	const AppTerm &asApp() const
	{
		return std::get<AppTerm>(m_Term);
	}

private:
	Kind m_Kind;
	Term_t m_Term;
};

class Program
{
public:
	Program() : m_Entry(nullptr) {}

	void setEntry(Term &&term)
	{
		m_Entry = std::make_shared<Term>(std::move(term));
	}

	std::weak_ptr<const Term> getEntry() const
	{
		return m_Entry;
	}

private:
	std::shared_ptr<Term> m_Entry;
};

class Parser
{
public:
	Parser();

	Program parse(const std::string &programSrc);

private:
	std::unordered_map<std::string, Term> parseFuncDefs();

	std::optional<Term> parseTerm(bool handleErrors = true);
	std::optional<AbsTerm> parseAbstraction(Loc loc = Loc(k_DefaultLoc));
	std::optional<AppTerm> parseApplication(Loc loc = Loc(k_DefaultLoc));

private:
	std::unique_ptr<Lexer> m_Lexer;
};