#pragma once

#include <memory>
#include <variant>
#include <string>

struct Loc
{
	Loc() : loc() {}
	Loc(const char *l) : loc(l) {}
	Loc(std::string l) : loc(l) {}

	friend bool operator==(const Loc &lhs, const Loc &rhs);
	const std::string &operator()() const;

	std::string loc;
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
	Var(const char *v) : var(v) {}
	Var(std::string v) : var(v) {}

	friend bool operator==(const Var &lhs, const Var &rhs);
	const std::string &operator()() const;

	std::string var;
};

struct VarHash
{
	std::size_t operator()(const Var &var) const
	{
		return std::hash<std::string>()(var.var);
	}
};

class Term;

struct NilTerm;
struct VarContTerm;
struct AbsTerm;
struct AppTerm;

struct NilTerm
{
};

struct VarContTerm
{
	VarContTerm();
	VarContTerm(Var var);

	Var var;
	std::unique_ptr<Term> body;
};

struct AbsTerm
{
	AbsTerm();
	AbsTerm(Loc loc, Var var);

	Loc loc;
	Var var;
	std::unique_ptr<Term> body;
};

struct AppTerm
{
	AppTerm();
	AppTerm(Loc loc);

	Loc loc;
	std::unique_ptr<Term> arg;
	std::unique_ptr<Term> body;
};

class Term
{
	using Term_t = std::variant<
		NilTerm, VarContTerm, AbsTerm, AppTerm
	>;

public:
	enum Kind
	{
		Nil, VarCont, Abs, App
	};

	Term();

	Term(NilTerm &&term);
	Term(VarContTerm &&term);
	Term(AbsTerm &&term);
	Term(AppTerm &&term);

	Kind kind() const;

	const NilTerm &asNil() const;
	const VarContTerm &asVarCont() const;
	const AbsTerm &asAbs() const;
	const AppTerm &asApp() const;

private:
	Kind m_Kind;
	Term_t m_Term;
};