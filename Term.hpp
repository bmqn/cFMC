#pragma once

#include <memory>
#include <variant>
#include <string>

#include "Config.hpp"

class Term;

struct NilTerm;
struct VarContTerm;
struct AbsTerm;
struct AppTerm;

struct ValTerm
{
	Val_t val;
};

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
	AbsTerm(Loc_t loc, Var_t var);

	Loc_t loc;
	Var_t var;
	std::unique_ptr<Term> body;
};

struct AppTerm
{
	AppTerm();
	AppTerm(Loc_t loc);

	Loc_t loc;
	std::unique_ptr<Term> arg;
	std::unique_ptr<Term> body;
};

class Term
{
	using Term_t = std::variant<
		ValTerm, NilTerm, VarContTerm, AbsTerm, AppTerm
	>;

public:
	enum Kind
	{
		Val, Nil, VarCont, Abs, App
	};

	Term();
	Term(const Term &term) = delete;
	Term(Term &&term) = default;

	Term(ValTerm &&term);
	Term(NilTerm &&term);
	Term(VarContTerm &&term);
	Term(AbsTerm &&term);
	Term(AppTerm &&term);

	Term &operator=(const Term &term) = delete;
	Term &operator=(Term &&term) = default;

	Kind kind() const;

	const ValTerm &asVal() const;
	const NilTerm &asNil() const;
	const VarContTerm &asVarCont() const;
	const AbsTerm &asAbs() const;
	const AppTerm &asApp() const;

private:
	Kind m_Kind;
	Term_t m_Term;
};