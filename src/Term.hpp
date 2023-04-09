#pragma once

#include <memory>
#include <optional>
#include <variant>
#include <string>
#include <map>

#include "Config.hpp"

class Term;

using TermOwner_t  = std::shared_ptr<Term>;
using TermHandle_t = std::shared_ptr<const Term>;

TermOwner_t newTerm(Term &&term);

class NilTerm
{
};

class VarTerm
{
public:
	VarTerm(const VarTerm &term) = delete;
	VarTerm(VarTerm &&term) = default;

	VarTerm(Var_t var);
	VarTerm(Var_t var, Term &&body);

	VarTerm &operator=(const VarTerm &term) = delete;
	VarTerm &operator=(VarTerm &&term) = delete;

	Var_t getVar() const;
	TermHandle_t getBody() const;

private:
	Var_t m_Var;
	TermOwner_t m_Body;
};

class AbsTerm
{
public:
	AbsTerm(const AbsTerm &term) = delete;
	AbsTerm(AbsTerm &&term) = default;

	AbsTerm(Loc_t loc, std::optional<Var_t> var);
	AbsTerm(Loc_t loc, std::optional<Var_t> var, Term &&body);

	AbsTerm &operator=(const AbsTerm &term) = delete;
	AbsTerm &operator=(AbsTerm &&term) = delete;

	Loc_t getLoc() const;
	std::optional<Var_t> getVar() const;
	TermHandle_t getBody() const;

private:
	Loc_t m_Loc;
	std::optional<Var_t> m_Var;
	TermOwner_t m_Body;
};

class AppTerm
{
public:
	AppTerm(const AppTerm &term) = delete;
	AppTerm(AppTerm &&term) = default;

	AppTerm(const Loc_t &loc, Term &&arg);
	AppTerm(const Loc_t &loc, Term &&arg, Term &&body);

	AppTerm &operator=(const AppTerm &term) = delete;
	AppTerm &operator=(AppTerm &&term) = delete;

	Loc_t getLoc() const;
	TermHandle_t getArg() const;
	TermHandle_t getBody() const;

private:
	Loc_t m_Loc;
	TermOwner_t m_Arg;
	TermOwner_t m_Body;
};

class LocAbsTerm
{
public:
	LocAbsTerm(const LocAbsTerm &term) = delete;
	LocAbsTerm(LocAbsTerm &&term) = default;

	LocAbsTerm(Loc_t loc, std::optional<LocVar_t> var);
	LocAbsTerm(Loc_t loc, std::optional<LocVar_t> var, Term &&body);

	LocAbsTerm &operator=(const LocAbsTerm &term) = delete;
	LocAbsTerm &operator=(LocAbsTerm &&term) = delete;

	Loc_t getLoc() const;
	std::optional<LocVar_t> getLocVar() const;
	TermHandle_t getBody() const;

private:
	Loc_t m_Loc;
	std::optional<LocVar_t> m_LocVar;
	TermOwner_t m_Body;
};

class LocAppTerm
{
public:
	LocAppTerm(const LocAppTerm &term) = delete;
	LocAppTerm(LocAppTerm &&term) = default;

	LocAppTerm(Loc_t loc, LocVar_t arg);
	LocAppTerm(Loc_t loc, LocVar_t arg, Term &&body);

	LocAppTerm &operator=(const LocAppTerm &term) = delete;
	LocAppTerm &operator=(LocAppTerm &&term) = delete;

	Loc_t getLoc() const;
	LocVar_t getArg() const;
	TermHandle_t getBody() const;

private:
	Loc_t m_Loc;
	LocVar_t m_Arg;
	TermOwner_t m_Body;
};

class ValTerm
{
public:
	ValTerm(const ValTerm &term) = delete;
	ValTerm(ValTerm &&term) = default;

	ValTerm(Prim_t prim);
	ValTerm(Loc_t loc);

	ValTerm &operator=(const ValTerm &term) = delete;
	ValTerm &operator=(ValTerm &&term) = delete;

	bool isPrim() const;
	bool isLoc() const;

	Prim_t asPrim() const;
	Loc_t asLoc() const;

private:
	std::variant<Prim_t, Loc_t> m_Val;
};

template<typename Case_t>
class CasesTerm
{
public:
	using Cases_t = std::map<Case_t, TermOwner_t>;

public:
	CasesTerm(const CasesTerm &cases) = delete;
	CasesTerm(CasesTerm &&cases) = default;

	CasesTerm(Cases_t &&cases, Term &&body);
	CasesTerm(Cases_t &&cases);

	CasesTerm &operator=(const CasesTerm &term) = delete;
	CasesTerm &operator=(CasesTerm &&term) = delete;

	TermHandle_t getBody() const;

	typename Cases_t::const_iterator find(const Case_t &c) const;
	typename Cases_t::const_iterator begin() const;
	typename Cases_t::const_iterator end() const;

private:
	Cases_t m_Cases;
	TermOwner_t m_Body;
};

class Term
{
public:
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
	Term &operator=(Term &&term) = delete;

	bool isNil() const;
	bool isVar() const;
	bool isAbs() const;
	bool isApp() const;
	bool isLocAbs() const;
	bool isLocApp() const;

	bool isVal() const;
	bool isLocCases() const;

	const NilTerm &asNil() const;
	const VarTerm &asVar() const;
	const AbsTerm &asAbs() const;
	const AppTerm &asApp() const;
	const LocAbsTerm &asLocAbs() const;
	const LocAppTerm &asLocApp() const;
	
	const ValTerm &asVal() const;
	const CasesTerm<Loc_t> &asLocCases() const;

private:
	std::variant<
		NilTerm, VarTerm, AbsTerm, AppTerm, LocAbsTerm, LocAppTerm, /* FCL-FMC    */
		ValTerm, CasesTerm<Loc_t>									/* Extensions */ 
	> m_Term;
};