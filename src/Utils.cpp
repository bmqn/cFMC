#include "Utils.hpp"

#include <sstream>
#include <iostream>

#include "Utils.hpp"

bool isReservedLoc(const Loc_t& loc)
{
	if      (loc == k_LambdaLoc)  { return true; }
	else if (loc == k_NewLoc)     { return true; }
	else if (loc == k_InputLoc)   { return true; }
	else if (loc == k_OutputLoc)  { return true; }
	else if (loc == k_NullLoc)    { return true; }

	return false;
}

std::optional<Loc_t> getReservedLocFromId(const std::string_view &id)
{
	if      (id == k_LambdaLoc)  { return k_LambdaLoc; }
	else if (id == k_NewLoc)     { return k_NewLoc; }
	else if (id == k_InputLoc)   { return k_InputLoc; }
	else if (id == k_OutputLoc)  { return k_OutputLoc; }
	else if (id == k_NullLoc)    { return k_NullLoc; }

	return std::nullopt;
}

std::optional<std::string> getIdFromReservedLoc(const Loc_t &loc)
{
	if      (loc == k_LambdaLoc)  { return std::string(k_LambdaLoc); }
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
			ss << "<@" << locAbs.getLocVar().value_or("_") << ">";
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
		else if (term->isPrimCases())
		{
			const CasesTerm<Prim_t> &cases = term->asPrimCases();
			ss << "(";
			for (auto itCases = cases.begin(); itCases != cases.end(); ++itCases)
			{
				ss << itCases->first;
				ss  << " -> " << stringifyTerm(itCases->second);
				ss << ", ";
			}
			ss << "otherwise -> ";
			ss << stringifyTerm(cases.getOtherwise());
			ss << ")";
			term = cases.getBody();
		}
		else if (term->isLocCases())
		{
			const CasesTerm<Loc_t> &cases = term->asLocCases();
			ss << "(";
			for (auto itCases = cases.begin(); itCases != cases.end(); ++itCases)
			{
				ss << itCases->first;
				ss  << " -> " << stringifyTerm(itCases->second);
				ss << ", ";
			}
			ss << "otherwise -> ";
			ss << stringifyTerm(cases.getOtherwise());
			ss << ")";
			term = cases.getBody();
		}
	}

	return ss.str();
}

std::string stringifyClosure(Closure_t closure, bool omitNil)
{
	std::stringstream ss;

	for (int i = 0; closure.second; ++i)
	{
		if (i > 0 && (!omitNil || !closure.second->isNil()))
		{
			ss << " . ";
		}

		if (closure.second->isNil())
		{
			if (!omitNil)
			{
				ss << "*";
			}

			closure.second = nullptr;
		}
		else if (closure.second->isVar())
		{
			const VarTerm &var = closure.second->asVar();

			auto itEnv = closure.first.first.find(var.getVar());
			if (itEnv != closure.first.first.end())
			{
				auto closurePtr = itEnv->second;
				ss << stringifyClosure(
					*reinterpret_cast<Closure_t *>(closurePtr.get())
				);
			}
			else
			{
				ss << var.getVar();
			}

			closure.second = var.getBody();
		}
		else if (closure.second->isApp())
		{
			const AppTerm &app = closure.second->asApp();
			
			ss << "[";
			ss << stringifyClosure(std::make_pair(
				closure.first, app.getArg())
			);
			ss << "]";

			auto itEnv = closure.first.second.find(app.getLoc());
			if (itEnv != closure.first.second.end())
			{
				if (itEnv->second != k_LambdaLoc)
				{
					ss << itEnv->second;
				}
			}
			else if (app.getLoc() != k_LambdaLoc)
			{
				ss << app.getLoc();
			}

			closure.second = app.getBody();
		}
		else if (closure.second->isAbs())
		{
			const AbsTerm &abs = closure.second->asAbs();

			{
				auto itEnv = closure.first.second.find(abs.getLoc());
				if (itEnv != closure.first.second.end())
				{
					if (itEnv->second != k_LambdaLoc)
					{
						ss << itEnv->second;
					}
				}
				else if (abs.getLoc() != k_LambdaLoc)
				{
					ss << abs.getLoc();
				}
			}

			ss << "<" << abs.getVar().value_or("_") << ">";

			if (abs.getVar())
			{
				auto itEnv = closure.first.first.find(abs.getVar().value());
				if (itEnv != closure.first.first.end())
				{
					closure.first.first.erase(itEnv);
				}
			}
			
			closure.second = abs.getBody();
		}
		else if (closure.second->isLocApp())
		{
			const LocAppTerm &locApp = closure.second->asLocApp();

			ss << "[#";
			{
				auto itEnv = closure.first.second.find(locApp.getArg());
				if (itEnv != closure.first.second.end())
				{
					ss << itEnv->second;
				}
				else
				{
					ss << locApp.getArg();
				}
			}
			ss << "]";

			{
				auto itEnv = closure.first.second.find(locApp.getLoc());
				if (itEnv != closure.first.second.end())
				{
					if (itEnv->second != k_LambdaLoc)
					{
						ss << itEnv->second;
					}
				}
				else if (locApp.getLoc() != k_LambdaLoc)
				{
					ss << locApp.getLoc();
				}
			}

			closure.second = locApp.getBody();
		}
		else if (closure.second->isLocAbs())
		{
			const LocAbsTerm &locAbs = closure.second->asLocAbs();

			{
				auto itEnv = closure.first.second.find(locAbs.getLoc());
				if (itEnv != closure.first.second.end())
				{
					if (itEnv->second != k_LambdaLoc)
					{
						ss << itEnv->second;
					}
				}
				else if (locAbs.getLoc() != k_LambdaLoc)
				{
					ss << locAbs.getLoc();
				}
			}

			if (locAbs.getLocVar())
			{
				auto itEnv = closure.first.second.find(locAbs.getLocVar().value());
				if (itEnv != closure.first.second.end())
				{
					closure.first.second.erase(itEnv);
				}
			}

			ss << "<@" << locAbs.getLocVar().value_or("_") << ">";

			closure.second = locAbs.getBody();
		}
		else if (closure.second->isVal())
		{
			const ValTerm &val = closure.second->asVal();

			if (val.isPrim())
			{
				ss << val.asPrim();
			}
			else if (val.isLoc())
			{
				ss << "#" << val.asLoc();
			}

			closure.second = nullptr;
		}
		else if (closure.second->isBinOp())
		{
			const BinOpTerm &binOp = closure.second->asBinOp();

			if (binOp.isOp(BinOpTerm::Plus))
			{
				ss << "+";
			}
			else if (binOp.isOp(BinOpTerm::Minus))
			{
				ss << "-";
			}

			closure.second = binOp.getBody();
		}
		else if (closure.second->isPrimCases())
		{
			const CasesTerm<Prim_t> &cases = closure.second->asPrimCases();

			ss << "(";
			for (auto itCases = cases.begin(); itCases != cases.end(); ++itCases)
			{
				ss << itCases->first;
				ss  << " -> ";
				ss << stringifyClosure(std::make_pair(
					closure.first, itCases->second)
				);
				ss << ", ";
			}
			ss << "otherwise -> ";
			ss << stringifyClosure(std::make_pair(
				closure.first, cases.getOtherwise())
			);
			ss << ")";

			closure.second = cases.getBody();
		}
		else if (closure.second->isLocCases())
		{
			const CasesTerm<Loc_t> &cases = closure.second->asLocCases();
			
			ss << "(";
			for (auto itCases = cases.begin(); itCases != cases.end(); ++itCases)
			{
				ss << itCases->first;
				ss  << " -> ";
				ss << stringifyClosure(std::make_pair(
					closure.first, itCases->second)
				);
				ss << ", ";
			}
			ss << "otherwise -> ";
			ss << stringifyClosure(std::make_pair(
				closure.first, cases.getOtherwise())
			);
			ss << ")";

			closure.second = cases.getBody();
		}
	}

	return ss.str();
}