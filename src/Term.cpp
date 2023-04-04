#include "Term.hpp"

TermOwner_t newTerm(Term &&term)
{
	return std::make_shared<Term>(std::move(term));
}

VarTerm::VarTerm(Var_t var)
	: m_Var(var)
	, m_Body(newTerm(NilTerm()))
{}

VarTerm::VarTerm(Var_t var, Term &&body)
	: m_Var(var)
	, m_Body(newTerm(std::move(body)))
{}

Var_t VarTerm::getVar() const
{
	return m_Var;
}

TermHandle_t VarTerm::getBody() const
{
	return m_Body;
}

AbsTerm::AbsTerm(Loc_t loc, std::optional<Var_t> var)
	: m_Loc(loc)
	, m_Var(var)
	, m_Body(newTerm(NilTerm()))
{
}

AbsTerm::AbsTerm(Loc_t loc, std::optional<Var_t> var, Term &&body)
	: m_Loc(loc)
	, m_Var(var)
	, m_Body(newTerm(std::move(body)))
{}

Loc_t AbsTerm::getLoc() const
{
	return m_Loc;
}

std::optional<Var_t> AbsTerm::getVar() const
{
	return m_Var;
}

TermHandle_t AbsTerm::getBody() const
{
	return m_Body;
}

AppTerm::AppTerm(const Loc_t &loc, Term &&arg)
	: m_Loc(loc)
	, m_Arg(newTerm(std::move(arg)))
	, m_Body(newTerm(NilTerm()))
{}

AppTerm::AppTerm(const Loc_t &loc, Term &&arg, Term &&body)
	: m_Loc(loc)
	, m_Arg(newTerm(std::move(arg)))
	, m_Body(newTerm(std::move(body)))
{}

Loc_t AppTerm::getLoc() const
{
	return m_Loc;
}

TermHandle_t AppTerm::getArg() const
{
	return m_Arg;
}

TermHandle_t AppTerm::getBody() const
{
	return m_Body;
}

ValTerm::ValTerm(Prim_t prim)
	: m_Val(prim)
{}

ValTerm::ValTerm(Loc_t loc)
	: m_Val(loc)
{}

LocAbsTerm::LocAbsTerm(Loc_t loc, std::optional<LocVar_t> var)
	: m_Loc(loc)
	, m_LocVar(var)
	, m_Body(newTerm(NilTerm()))
{}

LocAbsTerm::LocAbsTerm(Loc_t loc, std::optional<LocVar_t> var, Term &&body)
	: m_Loc(loc)
	, m_LocVar(var)
	, m_Body(newTerm(std::move(body)))
{}

Loc_t LocAbsTerm::getLoc() const
{
	return m_Loc;
}

std::optional<LocVar_t> LocAbsTerm::getLocVar() const
{
	return m_LocVar;
}

TermHandle_t LocAbsTerm::getBody() const
{
	return m_Body;
}

LocAppTerm::LocAppTerm(Loc_t loc, LocVar_t arg)
	: m_Loc(loc)
	, m_Arg(arg)
	, m_Body(newTerm(NilTerm()))
{}

LocAppTerm::LocAppTerm(Loc_t loc, LocVar_t arg, Term &&body)
	: m_Loc(loc)
	, m_Arg(arg)
	, m_Body(newTerm(std::move(body)))
{}

Loc_t LocAppTerm::getLoc() const
{
	return m_Loc;
}

LocVar_t LocAppTerm::getArg() const
{
	return m_Arg;
}

TermHandle_t LocAppTerm::getBody() const
{
	return m_Body;
}

bool ValTerm::isPrim() const
{
	return std::holds_alternative<Prim_t>(m_Val);
}

bool ValTerm::isLoc() const
{
	return std::holds_alternative<Loc_t>(m_Val);
}

Prim_t ValTerm::asPrim() const
{
	return std::get<Prim_t>(m_Val);
}

Loc_t ValTerm::asLoc() const
{
	return std::get<Loc_t>(m_Val);
}

template<typename Case_t>
CasesTerm<Case_t>::CasesTerm(CasesTerm<Case_t>::Cases_t &&cases, Term &&body)
	: m_Cases(std::move(cases))
	, m_Body(newTerm(std::move(body)))
{}

template<typename Case_t>
CasesTerm<Case_t>::CasesTerm(CasesTerm<Case_t>::Cases_t &&cases)
	: m_Cases(std::move(cases))
	, m_Body(newTerm(NilTerm()))
{}

template<typename Case_t>
TermHandle_t CasesTerm<Case_t>::getBody() const
{
	return m_Body;
}

template<typename Case_t>
typename CasesTerm<Case_t>::Cases_t::const_iterator CasesTerm<Case_t>::find(const Case_t &c) const
{
	return m_Cases.find(c);
}

template<typename Case_t>
typename CasesTerm<Case_t>::Cases_t::const_iterator CasesTerm<Case_t>::begin() const
{
	return m_Cases.begin();
}

template<typename Case_t>
typename CasesTerm<Case_t>::Cases_t::const_iterator CasesTerm<Case_t>::end() const
{
	return m_Cases.end();
}

template class CasesTerm<Loc_t>;

Term::Term()
	: m_Term(NilTerm())
{}

Term::Term(NilTerm &&term)
	: m_Term(std::move(term))
{}

Term::Term(VarTerm &&term)
	: m_Term(std::move(term))
{}

Term::Term(AbsTerm &&term)
	: m_Term(std::move(term))
{}

Term::Term(AppTerm &&term)
	: m_Term(std::move(term))
{}

Term::Term(LocAbsTerm &&term)
	: m_Term(std::move(term))
{}

Term::Term(LocAppTerm &&term)
	: m_Term(std::move(term))
{}

Term::Term(ValTerm &&term)
	: m_Term(std::move(term))
{}

Term::Term(CasesTerm<Loc_t> &&term)
	: m_Term(std::move(term))
{}

bool Term::isNil() const
{
	return std::holds_alternative<NilTerm>(m_Term);
}

bool Term::isVar() const
{
	return std::holds_alternative<VarTerm>(m_Term);
}

bool Term::isAbs() const
{
	return std::holds_alternative<AbsTerm>(m_Term);
}

bool Term::isApp() const
{
	return std::holds_alternative<AppTerm>(m_Term);
}

bool Term::isLocAbs() const
{
	return std::holds_alternative<LocAbsTerm>(m_Term);
}

bool Term::isLocApp() const
{
	return std::holds_alternative<LocAppTerm>(m_Term);
}

bool Term::isVal() const
{
	return std::holds_alternative<ValTerm>(m_Term);
}

bool Term::isLocCases() const
{
	return std::holds_alternative<CasesTerm<Loc_t>>(m_Term);
}

const NilTerm &Term::asNil() const
{
	return std::get<NilTerm>(m_Term);
}

const VarTerm &Term::asVar() const
{
	return std::get<VarTerm>(m_Term);
}

const AbsTerm &Term::asAbs() const
{
	return std::get<AbsTerm>(m_Term);
}

const AppTerm &Term::asApp() const
{
	return std::get<AppTerm>(m_Term);
}

const LocAbsTerm &Term::asLocAbs() const
{
	return std::get<LocAbsTerm>(m_Term);
}

const LocAppTerm &Term::asLocApp() const
{
	return std::get<LocAppTerm>(m_Term);
}

const ValTerm &Term::asVal() const
{
	return std::get<ValTerm>(m_Term);
}

const CasesTerm<Loc_t> &Term::asLocCases() const
{
	return std::get<CasesTerm<Loc_t>>(m_Term);
}