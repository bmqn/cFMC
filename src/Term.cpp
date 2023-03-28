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

VarContTerm::VarContTerm(Var_t var)
	: m_Var(var)
	, m_Body(newNilTerm())
{}

VarContTerm::VarContTerm(Var_t var, Term &&body)
	: m_Var(var)
	, m_Body(newNilTerm())
{
	*m_Body = std::move(body);
}

bool VarContTerm::is(Var_t var) const
{
	return m_Var == var;
}

Var_t VarContTerm::getVar() const
{
	return m_Var;
}

const Term *VarContTerm::getBody() const
{
	return m_Body.get();
}

AbsTerm::AbsTerm(Loc_t loc)
	: loc(loc)
	, var(std::nullopt)
	, body(newNilTerm())
{}

AbsTerm::AbsTerm(Loc_t loc, Var_t var)
	: loc(loc)
	, var(var)
	, body(newNilTerm())
{}

AppTerm::AppTerm(Var_t loc)
	: loc(loc)
	, arg(newNilTerm())
	, body(newNilTerm())
{}

ValTerm::ValTerm(Prim_t prim)
	: m_Kind(Prim)
	, m_Val(prim)
{
}

ValTerm::ValTerm(Loc_t loc)
	: m_Kind(Loc)
	, m_Val(loc)
{
}

LocAbsTerm::LocAbsTerm(Loc_t loc, LocVar_t var)
	: loc(loc)
	, var(var)
	, body(newNilTerm())
{}

LocAppTerm::LocAppTerm(Loc_t loc, Loc_t arg)
	: loc(loc)
	, arg(arg)
	, body(newNilTerm())
{}

ValTerm::ValKind ValTerm::kind() const
{
	return m_Kind;
}

Prim_t ValTerm::getPrim()  const
{
	return std::get<Prim_t>(m_Val);
}

Loc_t ValTerm::getLoc()  const
{
	return std::get<Loc_t>(m_Val);
}

CasesTerm::CasesTerm()
	: body(newNilTerm())
{}

void CasesTerm::addCase(std::string val, Term &&term)
{
	auto caseTerm = newNilTerm();
	*caseTerm = std::move(term);

	cases.emplace(val, std::move(caseTerm));
}

Term::Term()
	: m_Kind(Nil)
	, m_Term(NilTerm())
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

Term::Term(CasesTerm &&term)
	: m_Kind(Cases)
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

const CasesTerm &Term::asCases() const
{
	return std::get<CasesTerm>(m_Term);
}