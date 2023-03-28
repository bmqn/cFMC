#pragma once

#include <memory>
#include <optional>
#include <variant>
#include <string>
#include <map>

#include "Config.hpp"

class Term;

struct NilTerm
{
};

class VarContTerm
{
public:
	VarContTerm(Var_t var);
	VarContTerm(Var_t var, Term &&body);

	bool is(Var_t var) const;

	Var_t getVar() const;
	const Term *getBody() const;

private:
	Var_t m_Var;
	std::unique_ptr<Term> m_Body;
};

struct AbsTerm
{
	AbsTerm(Loc_t loc);
	AbsTerm(Loc_t loc, Var_t var);

	Loc_t loc;
	std::optional<Var_t> var;
	std::unique_ptr<Term> body;
};

struct AppTerm
{
	AppTerm(Loc_t loc);

	Loc_t loc;
	std::unique_ptr<Term> arg;
	std::unique_ptr<Term> body;
};

struct LocAbsTerm
{
	LocAbsTerm(Loc_t loc);
	LocAbsTerm(Loc_t loc, LocVar_t var);

	Loc_t loc;
	std::optional<LocVar_t> var;
	std::unique_ptr<Term> body;
};

struct LocAppTerm
{
	LocAppTerm(Loc_t loc, Loc_t arg);

	Loc_t loc;
	Loc_t arg;
	std::unique_ptr<Term> body;
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

struct CasesTerm
{
	CasesTerm();

	void addCase(std::string id, Term &&term);

	std::map<std::string, std::unique_ptr<Term>> cases;
	std::unique_ptr<Term> body;
};

class Term
{
	using Term_t = std::variant<
		NilTerm, VarContTerm, AbsTerm, AppTerm, LocAbsTerm, LocAppTerm,
		ValTerm, CasesTerm
	>;

public:
	enum Kind
	{
		/* FCL-FMC    */ Nil, VarCont, Abs, App, LocAbs, LocApp,
		/* Extensions */ Val, Cases
	};

	Term();
	Term(const Term &term) = delete;
	Term(Term &&term) = default;

	Term(NilTerm &&term);
	Term(VarContTerm &&term);
	Term(AbsTerm &&term);
	Term(AppTerm &&term);
	Term(LocAbsTerm &&term);
	Term(LocAppTerm &&term);
	Term(ValTerm &&term);
	Term(CasesTerm &&term);

	Term &operator=(const Term &term) = delete;
	Term &operator=(Term &&term) = default;

	Kind kind() const;

	const NilTerm &asNil() const;
	const VarContTerm &asVarCont() const;
	const AbsTerm &asAbs() const;
	const AppTerm &asApp() const;
	const LocAbsTerm &asLocAbs() const;
	const LocAppTerm &asLocApp() const;
	const ValTerm &asVal() const;
	const CasesTerm &asCases() const;

private:
	Kind m_Kind;
	Term_t m_Term;
};