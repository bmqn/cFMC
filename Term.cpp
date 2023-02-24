#include "Term.hpp"

static auto newNilTerm()
{
	return std::make_unique<Term>(Term(NilTerm()));
}

static auto newTerm(NilTerm &&term)
{
	return std::make_unique<Term>(Term(std::move(term)));
}

static auto newTerm(VarContTerm &&term)
{
	return std::make_unique<Term>(Term(std::move(term)));
}

static auto newTerm(AbsTerm &&term)
{
	return std::make_unique<Term>(Term(std::move(term)));
}

static auto newTerm(AppTerm &&term)
{
	return std::make_unique<Term>(Term(std::move(term)));
}

VarContTerm::VarContTerm()
	: var()
	, body(newNilTerm())
{}

VarContTerm::VarContTerm(Var_t var)
	: var(var)
	, body(newNilTerm())
{}

AbsTerm::AbsTerm()
	: loc()
	, var()
	, body(newNilTerm())
{}

AbsTerm::AbsTerm(Var_t loc, Var_t var)
	: loc(loc)
	, var(var)
	, body(newNilTerm())
{}

AppTerm::AppTerm()
	: loc()
	, arg(newNilTerm())
	, body(newNilTerm())
{}

AppTerm::AppTerm(Var_t loc)
	: loc(loc)
	, arg(newNilTerm())
	, body(newNilTerm())
{}

Term::Term()
	: m_Kind(Nil)
	, m_Term(NilTerm())
{}

Term::Term(ValTerm &&term)
	: m_Kind(Val)
	, m_Term(std::move(term))
{}

Term::Term(NilTerm &&term)
	: m_Kind(Nil)
	, m_Term(std::move(term))
{}

Term::Term(VarContTerm &&term)
	: m_Kind(VarCont)
	, m_Term(std::move(term))
{}

Term::Term(AbsTerm &&term)
	: m_Kind(Abs)
	, m_Term(std::move(term))
{}

Term::Term(AppTerm &&term)
	: m_Kind(App)
	, m_Term(std::move(term))
{}

Term::Kind Term::kind() const
{
	return m_Kind;
}

const ValTerm &Term::asVal() const
{
	return std::get<ValTerm>(m_Term);
}

const NilTerm &Term::asNil() const
{
	return std::get<NilTerm>(m_Term);
}

const VarContTerm &Term::asVarCont() const
{
	return std::get<VarContTerm>(m_Term);
}

const AbsTerm &Term::asAbs() const
{
	return std::get<AbsTerm>(m_Term);
}

const AppTerm &Term::asApp() const
{
	return std::get<AppTerm>(m_Term);
}