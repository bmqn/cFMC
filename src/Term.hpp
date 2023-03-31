#pragma once

#include <memory>
#include <optional>
#include <variant>
#include <string>
#include <map>

#include "Config.hpp"

class Term;

class NilTerm
{
};

class VarTerm
{
public:
	VarTerm(Var_t var);
	VarTerm(Var_t var, Term &&body);

	Var_t getVar() const;
	const Term *getBody() const;

	bool isVar(Var_t var) const;

private:
	Var_t m_Var;
	std::unique_ptr<Term> m_Body;
};

class AbsTerm
{
public:
	AbsTerm(Loc_t loc, std::optional<Var_t> var);
	AbsTerm(Loc_t loc, std::optional<Var_t> var, Term &&body);

	Loc_t getLoc() const;
	std::optional<Var_t> getVar() const;
	const Term *getBody() const;

private:
	Loc_t m_Loc;
	std::optional<Var_t> m_Var;
	std::unique_ptr<Term> m_Body;
};

class AppTerm
{
public:
	AppTerm(const Loc_t &loc, Term &&arg);
	AppTerm(const Loc_t &loc, Term &&arg, Term &&body);

	Loc_t getLoc() const;
	const Term *getArg() const;
	const Term *getBody() const;

private:
	Loc_t m_Loc;
	std::unique_ptr<Term> m_Arg;
	std::unique_ptr<Term> m_Body;
};

class LocAbsTerm
{
public:
	LocAbsTerm(Loc_t loc, std::optional<LocVar_t> var);
	LocAbsTerm(Loc_t loc, std::optional<LocVar_t> var, Term &&body);

	Loc_t getLoc() const;
	std::optional<LocVar_t> getLocVar() const;
	const Term *getBody() const;

private:
	Loc_t m_Loc;
	std::optional<LocVar_t> m_LocVar;
	std::unique_ptr<Term> m_Body;
};

class LocAppTerm
{
public:
	LocAppTerm(Loc_t loc, LocVar_t arg);
	LocAppTerm(Loc_t loc, LocVar_t arg, Term &&body);

	Loc_t getLoc() const;
	LocVar_t getArg() const;
	const Term *getBody() const;

private:
	Loc_t m_Loc;
	LocVar_t m_Arg;
	std::unique_ptr<Term> m_Body;
};

class ValTerm
{
public:
	enum ValKind
	{
		Prim, Loc
	};

	ValTerm(Prim_t prim);
	ValTerm(Loc_t loc);

	ValKind kind() const;

	Prim_t getPrim() const;
	Loc_t getLoc() const;

private:
	ValKind m_Kind;
	std::variant<Prim_t, Loc_t> m_Val;
};

template<typename Case_t>
class CasesTerm
{
public:
	using Cases_t = std::map<Case_t, std::unique_ptr<Term>>;

public:
	CasesTerm(const CasesTerm &cases) = delete;
	CasesTerm(CasesTerm &&cases) = default;

	CasesTerm(Cases_t &&cases, Term &&body);
	CasesTerm(Cases_t &&cases);

	CasesTerm &operator=(const CasesTerm &term) = delete;
	CasesTerm &operator=(CasesTerm &&term) = default;

	const Term *getBody() const;

	typename Cases_t::const_iterator find(const Case_t &c) const;
	typename Cases_t::const_iterator begin() const;
	typename Cases_t::const_iterator end() const;

private:
	Cases_t m_Cases;
	std::unique_ptr<Term> m_Body;
};

class Term
{
public:
	enum Kind
	{
		/* FCL-FMC    */ Nil, Var, Abs, App, LocAbs, LocApp,
		/* Extensions */ Val, LocCases
	};

	Term();
	Term(const Term &term) = delete;
	Term(Term &&term) = default;

	Term(NilTerm &&term);
	Term(VarTerm &&term);
	Term(AbsTerm &&term);
	Term(AppTerm &&term);
	Term(LocAbsTerm &&term);
	Term(LocAppTerm &&term);

	Term(ValTerm &&term);
	Term(CasesTerm<Loc_t> &&term);

	Term &operator=(const Term &term) = delete;
	Term &operator=(Term &&term) = default;

	Kind kind() const;

	const NilTerm &asNil() const;
	const VarTerm &asVar() const;
	const AbsTerm &asAbs() const;
	const AppTerm &asApp() const;
	const LocAbsTerm &asLocAbs() const;
	const LocAppTerm &asLocApp() const;
	
	const ValTerm &asVal() const;
	const CasesTerm<Loc_t> &asLocCases() const;

private:
	using Term_t = std::variant<
		/* FCL-FMC    */ NilTerm, VarTerm, AbsTerm, AppTerm, LocAbsTerm, LocAppTerm,
		/* Extensions */ ValTerm, CasesTerm<Loc_t>
	>;

	Kind m_Kind;
	Term_t m_Term;
};