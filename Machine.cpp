#include "Machine.hpp"

#include <sstream>

std::string stringifyTerm(const Term &term)
{
	std::stringstream termSs;
	const Term *termPtr = nullptr;

	switch (term.kind())
	{
	case Term::Nil:
	{
		termSs << "*";
		termPtr = nullptr;
		break;
	}
	case Term::VarCont:
	{
		const VarContTerm &varCont = term.asVarCont();
		termSs << varCont.var.var << " . ";
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
		termSs << "<" << abs.var.var << "> . ";
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
		termSs << "[" << stringifyTerm(*app.arg) << "] . ";
		termPtr = app.body.get();
		break;
	}
	}

	while (termPtr)
	{
		switch (termPtr->kind())
		{
		case Term::Nil:
		{
			termSs << "*";
			termPtr = nullptr;
			break;
		}
		case Term::VarCont:
		{
			const VarContTerm &varCont = termPtr->asVarCont();
			termSs << varCont.var.var << " . ";
			termPtr = varCont.body.get();
			break;
		}
		case Term::Abs:
		{
			const AbsTerm &abs = termPtr->asAbs();
			if (abs.loc != k_DefaultLoc)
			{
				termSs << abs.loc.loc;
			}
			termSs << "<" << abs.var.var << "> . ";
			termPtr = abs.body.get();
			break;
		}
		case Term::App:
		{
			const AppTerm &app = termPtr->asApp();
			if (app.loc != k_DefaultLoc)
			{
				termSs << app.loc.loc;
			}
			termSs << "[" << stringifyTerm(*app.arg) << "] . ";
			termPtr = app.body.get();
			break;
		}
		}
	}

	return termSs.str();
}

Machine::Machine()
{
	static Term zero(VarContTerm(Var(std::string_view("0"))));
	static Term two(VarContTerm(Var(std::string_view("2"))));
	static Term seven(VarContTerm(Var(std::string_view("7"))));
	static Term nine(VarContTerm(Var(std::string_view("9"))));

	m_Stacks[std::string(k_DefaultLoc)].push_back(&zero);
	m_Stacks[std::string(k_RandomLoc)].push_back(&two);
	m_Stacks[std::string(k_RandomLoc)].push_back(&seven);
	m_Stacks[std::string(k_RandomLoc)].push_back(&nine);
}

void Machine::execute(Program &program)
{
	if (auto entryPtr = program.getEntry().lock())
	{
		std::stack<std::pair<const Term *, BoundVars_t>> frame;
		frame.push(std::make_pair(entryPtr.get(), BoundVars_t()));

		std::unordered_map<const Term *, BoundVars_t> env;

		while (!frame.empty())
		{
			auto &[term, bound] = frame.top();

			switch (term->kind())
			{
			case Term::Nil:
			{
				frame.pop();

				break;
			}
			case Term::VarCont:
			{
				const VarContTerm &varCont = term->asVarCont();

				term = varCont.body.get();

				auto itBound = bound.find(varCont.var);
				if (itBound != bound.end())
				{
					auto ItEnv = env.find(itBound->second);
					if (ItEnv != env.end())
					{
						frame.push(std::make_pair(itBound->second, ItEnv->second));
					}
					else
					{
						std::cout << "[INFO] Variable '" << varCont.var.var << "' "
							<< "is bound to '" << stringifyTerm(*itBound->second) << "' "
							<< "but did not have an environment binding ! "
							<< "Be weary of variable capture !"
							<< std::endl;

						frame.push(std::make_pair(itBound->second, bound));
					}
				}
				else
				{
					std::cout << "[INFO] Variable '" << varCont.var.var << "' "
						<< "is not bound to anything !"
						<< std::endl;
				}

				break;
			}
			case Term::Abs:
			{
				const AbsTerm &abs = term->asAbs();

				term = abs.body.get();

				bound[abs.var] = m_Stacks[abs.loc].back();
				m_Stacks[abs.loc].pop_back();

				break;
			}
			case Term::App:
			{
				const AppTerm &app = term->asApp();
				
				term = app.body.get();

				const Term *toPush = app.arg.get();
				if (app.arg->kind() == Term::VarCont)
				{
					const VarContTerm &varCont = app.arg->asVarCont();
					auto it = bound.find(varCont.var);
					if (it != bound.end())
					{
						toPush = it->second;
					}
				}
				m_Stacks[app.loc].push_back(toPush);
				env[toPush] = bound;

				break;
			}
			}
		}
	}
	else
	{
		std::cerr << "[Machine Error] Given program has an invalid (expired) program !" << std::endl;
		return;
	}
}

void Machine::printDebug()
{
	std::cout << "------------------" << std::endl;
	std::cout << "      Debug       " << std::endl;
	std::cout << "------------------" << std::endl;
	std::cout << "--- Stacks ---" << std::endl;

	for (auto itStacks = m_Stacks.begin(); itStacks != m_Stacks.end(); ++itStacks)
	{
		std::cout << itStacks->first.loc << std::endl;

		for (auto itStack = itStacks->second.rbegin(); itStack != itStacks->second.rend(); ++itStack)
		{
			const auto &term = *itStack;
			std::cout << "    " << stringifyTerm(*term) << std::endl;
		}

		auto itStacksCopy = itStacks;
		if (!(++itStacksCopy == m_Stacks.end()))
		{
			std::cout << std::endl;
		}
	}

	std::cout << "------------------" << std::endl;
}