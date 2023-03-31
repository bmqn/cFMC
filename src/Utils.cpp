#include "Utils.hpp"

#include <sstream>
#include <iostream>

#include "Utils.hpp"

std::optional<Loc_t> getReservedLocFromId(const std::string_view &id)
{
	if      (id == k_DefaultLoc) { return Loc_t(k_DefaultLoc); }
	else if (id == k_NewLoc)     { return Loc_t(k_NewLoc); }
	else if (id == k_InputLoc)   { return Loc_t(k_InputLoc); }
	else if (id == k_OutputLoc)  { return Loc_t(k_OutputLoc); }
	else if (id == k_NullLoc)    { return Loc_t(k_NullLoc); }

	return std::nullopt;
}

std::optional<std::string> getIdFromReservedLoc(const Loc_t &loc)
{
	if      (loc == k_DefaultLoc) { return std::string(k_DefaultLoc); }
	else if (loc == k_NewLoc)     { return std::string(k_NewLoc); }
	else if (loc == k_InputLoc)   { return std::string(k_InputLoc); }
	else if (loc == k_OutputLoc)  { return std::string(k_OutputLoc); }
	else if (loc == k_NullLoc)    { return std::string(k_NullLoc); }

	return std::nullopt;
}

std::string stringifyTerm(const Term &term, bool omitNil)
{
	std::stringstream termSs;
	const Term *termPtr = nullptr;

	switch (term.kind())
	{
	case Term::Nil:
	{
		if (!omitNil)
		{
			termSs << "*";
		}
		termPtr = nullptr;
		break;
	}
	case Term::Var:
	{
		const VarTerm &var = term.asVar();
		termSs << var.getVar();
		termPtr = var.getBody();
		break;
	}
	case Term::Abs:
	{
		const AbsTerm &abs = term.asAbs();
		if (abs.getLoc() != k_DefaultLoc)
		{
			termSs << abs.getLoc();
		}
		termSs << "<" << abs.getVar().value_or("_") << ">";
		termPtr = abs.getBody();
		break;
	}
	case Term::App:
	{
		const AppTerm &app = term.asApp();
		termSs << "[" << stringifyTerm(*app.getArg()) << "]";
		if (app.getLoc() != k_DefaultLoc)
		{
			termSs << app.getLoc();
		}
		termPtr = app.getBody();
		break;
	}
	case Term::LocAbs:
	{
		const LocAbsTerm &locAbs = term.asLocAbs();
		if (locAbs.getLoc() != k_DefaultLoc)
		{
			termSs << locAbs.getLoc();
		}
		termSs << "<^" << locAbs.getLocVar().value_or("_") << ">";
		termPtr = locAbs.getBody();
		break;
	}
	case Term::LocApp:
	{
		const LocAppTerm &locApp = term.asLocApp();
		termSs << "[#" << locApp.getArg() << "]";
		if (locApp.getLoc() != k_DefaultLoc)
		{
			termSs << locApp.getLoc();
		}
		termPtr = locApp.getBody();
		break;
	}
	case Term::Val:
	{
		const ValTerm &val = term.asVal();
		if (val.kind() == ValTerm::Prim)
		{
			termSs << static_cast<uint32_t>(val.getPrim());
		}
		else if (val.kind() == ValTerm::Loc)
		{
			termSs << std::string(val.getLoc());
		}
		termPtr = nullptr;
		break;
	}
	case Term::LocCases:
	{
		const CasesTerm<Loc_t> &cases = term.asLocCases();
		termSs << "(";
		for (auto itCases = cases.begin(); itCases != cases.end(); ++itCases)
		{
			termSs << itCases->first;
			termSs << " -> " << stringifyTerm(*itCases->second.get());

			auto itCasesCopy = itCases;
			if (!(++itCasesCopy == cases.end()))
			{
				termSs << ", ";
			}
		}
		termSs << ")";
		termPtr = cases.getBody();
		break;
	}
	}

	while (termPtr)
	{
		switch (termPtr->kind())
		{
		case Term::Nil:
		{
			if (!omitNil)
			{
				termSs << " . *";
			}
			termPtr = nullptr;
			break;
		}
		case Term::Var:
		{
			const VarTerm &var = termPtr->asVar();
			termSs << " . " << var.getVar();
			termPtr = var.getBody();
			break;
		}
		case Term::Abs:
		{
			const AbsTerm &abs = termPtr->asAbs();
			termSs << " . ";
			if (abs.getLoc() != k_DefaultLoc)
			{
				termSs << abs.getLoc();
			}
			termSs << "<" << abs.getVar().value_or("_") << ">";
			termPtr = abs.getBody();
			break;
		}
		case Term::App:
		{
			const AppTerm &app = termPtr->asApp();
			termSs << " . ";
			termSs << "[" << stringifyTerm(*app.getArg()) << "]";
			if (app.getLoc() != k_DefaultLoc)
			{
				termSs << app.getLoc();
			}
			termPtr = app.getBody();
			break;
		}
		case Term::LocAbs:
		{
			const LocAbsTerm &locAbs = termPtr->asLocAbs();
			termSs << " . ";
			if (locAbs.getLoc() != k_DefaultLoc)
			{
				termSs << locAbs.getLoc();
			}
			termSs << "<^" << locAbs.getLocVar().value_or("_") << ">";
			termPtr = locAbs.getBody();
			break;
		}
		case Term::LocApp:
		{
			const LocAppTerm &locApp = termPtr->asLocApp();
			termSs << " . ";
			termSs << "[#" << locApp.getArg() << "]";
			if (locApp.getLoc() != k_DefaultLoc)
			{
				termSs << locApp.getLoc();
			}
			termPtr = locApp.getBody();
			break;
		}
		case Term::Val:
		{
			const ValTerm &val = term.asVal();
			if (val.kind() == ValTerm::Prim)
			{
				termSs << static_cast<uint32_t>(val.getPrim());
			}
			else if (val.kind() == ValTerm::Loc)
			{
				termSs << std::string(val.getLoc());
			}
			termPtr = nullptr;
			break;
		}
		case Term::LocCases:
		{
			const CasesTerm<Loc_t> &cases = termPtr->asLocCases();
			termSs << ". (";
			for (auto itCases = cases.begin(); itCases != cases.end(); ++itCases)
			{
				termSs << itCases->first;
				termSs  << " -> " << stringifyTerm(*itCases->second.get());

				auto itCasesCopy = itCases;
				if (!(++itCasesCopy == cases.end()))
				{
					termSs << ", ";
				}
			}
			termSs << ")";
			termPtr = cases.getBody();
			break;
		}
		}
	}

	return termSs.str();
}