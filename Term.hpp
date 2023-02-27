#pragma once

#include <memory>
#include <variant>
#include <string>
#include <map>

#include "Config.hpp"

class Term;

struct NilTerm
{
};

struct VarContTerm
{
	VarContTerm();
	VarContTerm(Var_t var);

	Var_t var;
	std::unique_ptr<Term> body;
};

struct AbsTerm
{
	AbsTerm();
	AbsTerm(Var_t loc, Var_t var);

	Var_t loc;
	Var_t var;
	std::unique_ptr<Term> body;
};

struct AppTerm
{
	AppTerm();
	AppTerm(Var_t loc);

	Var_t loc;
	std::unique_ptr<Term> arg;
	std::unique_ptr<Term> body;
};

struct ValTerm
{
	Val_t val;
};

struct CasesTerm
{
	CasesTerm();

	void addCase(Val_t val, Term &&term);

	std::map<Val_t, std::unique_ptr<Term>> cases;
	std::unique_ptr<Term> body;
};

class Term
{
	using Term_t = std::variant<
		NilTerm, VarContTerm, AbsTerm, AppTerm, ValTerm, CasesTerm
	>;

public:
	enum Kind
	{
		Nil, VarCont, Abs, App, Val, Cases
	};

	Term();
	Term(const Term &term) = delete;
	Term(Term &&term) = default;

	Term(NilTerm &&term);
	Term(VarContTerm &&term);
	Term(AbsTerm &&term);
	Term(AppTerm &&term);
	Term(ValTerm &&term);
	Term(CasesTerm &&term);

	Term &operator=(const Term &term) = delete;
	Term &operator=(Term &&term) = default;

	Kind kind() const;

	const NilTerm &asNil() const;
	const VarContTerm &asVarCont() const;
	const AbsTerm &asAbs() const;
	const AppTerm &asApp() const;
	const ValTerm &asVal() const;
	const CasesTerm &asCases() const;

private:
	Kind m_Kind;
	Term_t m_Term;
};