#include "Utils.hpp"

#include <sstream>

#include "Utils.hpp"
#include "Config.hpp"

std::string stringifyTerm(const Term &term, bool omitNil)
{
	std::stringstream termSs;
	const Term *termPtr = nullptr;

	switch (term.kind())
	{
	case Term::Val:
	{
		const ValTerm &val = term.asVal();
		termSs << val.val;
		termPtr = nullptr;
		break;
	}
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
		termSs << varCont.var.var;
		termPtr = varCont.body.get();
		break;
	}
	case Term::Abs:
	{
		const AbsTerm &abs = term.asAbs();
		if (abs.loc != k_DefaultLoc)
		{
			termSs << abs.loc.loc;
		}
		termSs << "<" << abs.var.var << ">";
		termPtr = abs.body.get();
		break;
	}
	case Term::App:
	{
		const AppTerm &app = term.asApp();
		if (app.loc != k_DefaultLoc)
		{
			termSs << app.loc.loc;
		}
		termSs << "[" << stringifyTerm(*app.arg) << "]";
		termPtr = app.body.get();
		break;
	}
	}

	while (termPtr)
	{
		switch (termPtr->kind())
		{
		case Term::Val:
		{
			const ValTerm &val = term.asVal();
			termSs << " . " << val.val;
			termPtr = nullptr;
			break;
		}
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
			termSs << " . " << varCont.var.var;
			termPtr = varCont.body.get();
			break;
		}
		case Term::Abs:
		{
			const AbsTerm &abs = termPtr->asAbs();
			termSs << " . ";
			if (abs.loc != k_DefaultLoc)
			{
				termSs << abs.loc.loc;
			}
			termSs << "<" << abs.var.var << ">";
			termPtr = abs.body.get();
			break;
		}
		case Term::App:
		{
			const AppTerm &app = termPtr->asApp();
			termSs << " . ";
			if (app.loc != k_DefaultLoc)
			{
				termSs << app.loc.loc;
			}
			termSs << "[" << stringifyTerm(*app.arg) << "]";
			termPtr = app.body.get();
			break;
		}
		}
	}

	return termSs.str();
}