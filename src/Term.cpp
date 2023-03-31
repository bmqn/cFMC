#include "Term.hpp"

static auto newNilTerm()
{
	return std::make_unique<Term>(Term(NilTerm()));
}

static auto newTerm(NilTerm &&term)
{
	return std::make_unique<Term>(Term(std::move(term)));
}

static auto newTerm(VarTerm &&term)
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

VarTerm::VarTerm(Var_t var)
	: m_Var(var)
	, m_Body(newNilTerm())
{}

VarTerm::VarTerm(Var_t var, Term &&body)
	: m_Var(var)
	, m_Body(newNilTerm())
{
	*m_Body = std::move(body);
}

Var_t VarTerm::getVar() const
{
	return m_Var;
}

const Term *VarTerm::getBody() const
{
	return m_Body.get();
}

bool VarTerm::isVar(Var_t var) const
{
	return m_Var == var;
}

AbsTerm::AbsTerm(Loc_t loc, std::optional<Var_t> var)
	: m_Loc(loc)
	, m_Var(var)
	, m_Body(newNilTerm())
{
}

AbsTerm::AbsTerm(Loc_t loc, std::optional<Var_t> var, Term &&body)
	: m_Loc(loc)
	, m_Var(var)
	, m_Body(newNilTerm())
{
	*m_Body = std::move(body);
}

Loc_t AbsTerm::getLoc() const
{
	return m_Loc;
}

std::optional<Var_t> AbsTerm::getVar() const
{
	return m_Var;
}

const Term *AbsTerm::getBody() const
{
	return m_Body.get();
}

AppTerm::AppTerm(const Loc_t &loc, Term &&arg)
	: m_Loc(loc)
	, m_Arg(newNilTerm())
	, m_Body(newNilTerm())
{
	*m_Arg = std::move(arg);
}

AppTerm::AppTerm(const Loc_t &loc, Term &&arg, Term &&body)
	: m_Loc(loc)
	, m_Arg(newNilTerm())
	, m_Body(newNilTerm())
{
	*m_Arg = std::move(arg);
	*m_Body = std::move(body);
}

Loc_t AppTerm::getLoc() const
{
	return m_Loc;
}

const Term *AppTerm::getArg() const
{
	return m_Arg.get();
}

const Term *AppTerm::getBody() const
{
	return m_Body.get();
}

ValTerm::ValTerm(Prim_t prim)
	: m_Kind(Prim)
	, m_Val(prim)
{}

ValTerm::ValTerm(Loc_t loc)
	: m_Kind(Loc)
	, m_Val(loc)
{}

LocAbsTerm::LocAbsTerm(Loc_t loc, std::optional<LocVar_t> var)
	: m_Loc(loc)
	, m_LocVar(var)
	, m_Body(newNilTerm())
{}

LocAbsTerm::LocAbsTerm(Loc_t loc, std::optional<LocVar_t> var, Term &&body)
	: m_Loc(loc)
	, m_LocVar(var)
	, m_Body(newNilTerm())
{
	*m_Body = std::move(body);
}

Loc_t LocAbsTerm::getLoc() const
{
	return m_Loc;
}

std::optional<LocVar_t> LocAbsTerm::getLocVar() const
{
	return m_LocVar;
}

const Term *LocAbsTerm::getBody() const
{
	return m_Body.get();
}

LocAppTerm::LocAppTerm(Loc_t loc, LocVar_t arg)
	: m_Loc(loc)
	, m_Arg(arg)
	, m_Body(newNilTerm())
{}

LocAppTerm::LocAppTerm(Loc_t loc, LocVar_t arg, Term &&body)
	: m_Loc(loc)
	, m_Arg(arg)
	, m_Body(newNilTerm())
{
	*m_Body = std::move(body); 
}

Loc_t LocAppTerm::getLoc() const
{
	return m_Loc;
}

LocVar_t LocAppTerm::getArg() const
{
	return m_Arg;
}

const Term *LocAppTerm::getBody() const
{
	return m_Body.get();
}

ValTerm::ValKind ValTerm::kind() const
{
	return m_Kind;
}

Prim_t ValTerm::getPrim() const
{
	return std::get<Prim_t>(m_Val);
}

Loc_t ValTerm::getLoc() const
{
	return std::get<Loc_t>(m_Val);
}

template<typename Case_t>
CasesTerm<Case_t>::CasesTerm(CasesTerm<Case_t>::Cases_t &&cases, Term &&body)
	: m_Cases(std::move(cases))
	, m_Body(newNilTerm())
{
	*m_Body = std::move(body);
}

template<typename Case_t>
CasesTerm<Case_t>::CasesTerm(CasesTerm<Case_t>::Cases_t &&cases)
	: m_Cases(std::move(cases))
	, m_Body(newNilTerm())
{}

template<typename Case_t>
const Term *CasesTerm<Case_t>::getBody() const
{
	return m_Body.get();
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
	: m_Kind(Nil)
	, m_Term(NilTerm())
{}

Term::Term(NilTerm &&term)
	: m_Kind(Nil)
	, m_Term(std::move(term))
{}

Term::Term(VarTerm &&term)
	: m_Kind(Var)
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

Term::Term(LocAbsTerm &&term)
	: m_Kind(LocAbs)
	, m_Term(std::move(term))
{}

Term::Term(LocAppTerm &&term)
	: m_Kind(LocApp)
	, m_Term(std::move(term))
{}

Term::Term(ValTerm &&term)
	: m_Kind(Val)
	, m_Term(std::move(term))
{}

Term::Term(CasesTerm<Loc_t> &&term)
	: m_Kind(LocCases)
	, m_Term(std::move(term))
{}

Term::Kind Term::kind() const
{
	return m_Kind;
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