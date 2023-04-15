#include "Utils.hpp"

#include <sstream>
#include <iostream>

#include "Utils.hpp"

std::optional<Loc_t> getReservedLocFromId(const std::string_view &id)
{
	if      (id == k_LambdaLoc) { return k_LambdaLoc; }
	else if (id == k_NewLoc)     { return k_NewLoc; }
	else if (id == k_InputLoc)   { return k_InputLoc; }
	else if (id == k_OutputLoc)  { return k_OutputLoc; }
	else if (id == k_NullLoc)    { return k_NullLoc; }

	return std::nullopt;
}

std::optional<std::string> getIdFromReservedLoc(const Loc_t &loc)
{
	if      (loc == k_LambdaLoc) { return std::string(k_LambdaLoc); }
	else if (loc == k_NewLoc)     { return std::string(k_NewLoc); }
	else if (loc == k_InputLoc)   { return std::string(k_InputLoc); }
	else if (loc == k_OutputLoc)  { return std::string(k_OutputLoc); }
	else if (loc == k_NullLoc)    { return std::string(k_NullLoc); }

	return std::nullopt;
}

std::string stringifyTerm(TermHandle_t term, bool omitNil)
{
	std::stringstream ss;

	for (int i = 0; term; ++i)
	{
		if (i > 0 && (!omitNil || !term->isNil()))
		{
			ss << " . ";
		}

		if (term->isNil())
		{
			if (!omitNil)
			{
				ss << "*";
			}
			term = nullptr;
		}
		else if (term->isVar())
		{
			const VarTerm &var = term->asVar();
			ss << var.getVar();
			term = var.getBody();
		}
		else if (term->isAbs())
		{
			const AbsTerm &abs = term->asAbs();
			if (abs.getLoc() != k_LambdaLoc)
			{
				ss << abs.getLoc();
			}
			ss << "<" << abs.getVar().value_or("_") << ">";
			term = abs.getBody();
		}
		else if (term->isApp())
		{
			const AppTerm &app = term->asApp();
			ss << "[" << stringifyTerm(app.getArg()) << "]";
			if (app.getLoc() != k_LambdaLoc)
			{
				ss << app.getLoc();
			}
			term = app.getBody();
		}
		else if (term->isLocAbs())
		{
			const LocAbsTerm &locAbs = term->asLocAbs();
			if (locAbs.getLoc() != k_LambdaLoc)
			{
				ss << locAbs.getLoc();
			}
			ss << "<^" << locAbs.getLocVar().value_or("_") << ">";
			term = locAbs.getBody();
		}
		else if (term->isLocApp())
		{
			const LocAppTerm &locApp = term->asLocApp();
			ss << "[#" << locApp.getArg() << "]";
			if (locApp.getLoc() != k_LambdaLoc)
			{
				ss << locApp.getLoc();
			}
			term = locApp.getBody();
		}
		else if (term->isVal())
		{
			const ValTerm &val = term->asVal();
			if (val.isPrim())
			{
				ss << val.asPrim();
			}
			else if (val.isLoc())
			{
				ss << "#" << val.asLoc();
			}
			term = nullptr;
		}
		else if (term->isBinOp())
		{
			const BinOpTerm &binOp = term->asBinOp();
			if (binOp.isOp(BinOpTerm::Plus))
			{
				ss << "+";
			}
			else if (binOp.isOp(BinOpTerm::Minus))
			{
				ss << "-";
			}
			term = binOp.getBody();
		}
		else if (term->isLocCases())
		{
			const CasesTerm<Loc_t> &cases = term->asLocCases();
			ss << "(";
			for (auto itCases = cases.begin(); itCases != cases.end(); ++itCases)
			{
				ss << itCases->first;
				ss  << " -> " << stringifyTerm(itCases->second);

				auto itCasesCopy = itCases;
				if (!(++itCasesCopy == cases.end()))
				{
					ss << ", ";
				}
			}
			ss << ")";
			term = cases.getBody();
		}
	}

	return ss.str();
}