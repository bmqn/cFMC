#include "Utils.hpp"

#include <sstream>

#include "Utils.hpp"

std::optional<Loc_t> getReservedLocFromId(const std::string_view &id)
{
	if      (id == k_DefaultLoc) { return Loc_t(k_DefaultLoc); }
	else if (id == k_NewLoc)     { return Loc_t(k_NewLoc); }
	else if (id == k_InputLoc)   { return Loc_t(k_InputLoc); }
	else if (id == k_OutputLoc)  { return Loc_t(k_OutputLoc); }

	return std::nullopt;
}

std::optional<std::string> getIdFromReservedLoc(const Loc_t &loc)
{
	if      (loc == k_DefaultLoc) { return std::string(k_DefaultLoc); }
	else if (loc == k_NewLoc)     { return std::string(k_NewLoc); }
	else if (loc == k_InputLoc)   { return std::string(k_InputLoc); }
	else if (loc == k_OutputLoc)  { return std::string(k_OutputLoc); }

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
	case Term::VarCont:
	{
		const VarContTerm &varCont = term.asVarCont();
		termSs << varCont.getVar();
		termPtr = varCont.getBody();
		break;
	}
	case Term::Abs:
	{
		const AbsTerm &abs = term.asAbs();
		if (abs.loc != k_DefaultLoc)
		{
			termSs << abs.loc;
		}
		termSs << "<" << abs.var.value_or("_") << ">";
		termPtr = abs.body.get();
		break;
	}
	case Term::App:
	{
		const AppTerm &app = term.asApp();
		if (app.loc != k_DefaultLoc)
		{
			termSs << app.loc;
		}
		termSs << "[" << stringifyTerm(*app.arg) << "]";
		termPtr = app.body.get();
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
	case Term::Cases:
	{
		// const CasesTerm &cases = term.asCases();
		// termSs << "(";
		// for (auto itCases = cases.cases.begin(); itCases != cases.cases.end(); ++itCases)
		// {
		// 	if (itCases->first == static_cast<Val_t>(-1))
		// 	{
		// 		termSs << "otherwise";
		// 	}
		// 	else
		// 	{
		// 		termSs << static_cast<uint32_t>(itCases->first);
		// 	}
		// 	termSs  << " -> " << stringifyTerm(*itCases->second);

		// 	auto itCasesCopy = itCases;
		// 	if (!(++itCasesCopy == cases.cases.end()))
		// 	{
		// 		termSs << ", ";
		// 	}
		// }
		// termSs << ")";
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
		case Term::VarCont:
		{
			const VarContTerm &varCont = termPtr->asVarCont();
			termSs << " . " << varCont.getVar();
			termPtr = varCont.getBody();
			break;
		}
		case Term::Abs:
		{
			const AbsTerm &abs = termPtr->asAbs();
			termSs << " . ";
			if (abs.loc != k_DefaultLoc)
			{
				termSs << abs.loc;
			}
			termSs << "<" << abs.var.value_or("_") << ">";
			termPtr = abs.body.get();
			break;
		}
		case Term::App:
		{
			const AppTerm &app = termPtr->asApp();
			termSs << " . ";
			if (app.loc != k_DefaultLoc)
			{
				termSs << app.loc;
			}
			termSs << "[" << stringifyTerm(*app.arg) << "]";
			termPtr = app.body.get();
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
		case Term::Cases:
		{
			// const CasesTerm &cases = term.asCases();
			// termSs << ". (";
			// for (auto itCases = cases.cases.begin(); itCases != cases.cases.end(); ++itCases)
			// {
			// 	if (itCases->first == static_cast<Val_t>(-1))
			// 	{
			// 		termSs << "otherwise";
			// 	}
			// 	else
			// 	{
			// 		termSs << static_cast<uint32_t>(itCases->first);
			// 	}
			// 	termSs  << " -> " << stringifyTerm(*itCases->second);

			// 	auto itCasesCopy = itCases;
			// 	if (!(++itCasesCopy == cases.cases.end()))
			// 	{
			// 		termSs << ", ";
			// 	}
			// }
			// termSs << ")";
			break;
		}
		}
	}

	return termSs.str();
}