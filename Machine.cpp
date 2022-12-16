#include "Machine.hpp"

Scope_t createScope(const Term *term)
{
	return std::make_pair(term, BoundVars_t());
}

Machine::Machine()
{
	static Term zero(Term::VarCont, VarContTerm(Var("0")));
	static Term two(Term::VarCont, VarContTerm(Var("2")));
	static Term seven(Term::VarCont, VarContTerm(Var("7")));

	m_Stacks[std::string(k_DefaultLoc)].push_back(&zero);
	m_Stacks[std::string(k_RandomLoc)].push_back(&two);
	m_Stacks[std::string(k_RandomLoc)].push_back(&seven);
}

void Machine::execute(const Program &program)
{
	m_Scope.push(createScope(program.getEntry()));

	while (!m_Scope.empty())
	{
		auto &[term, boundVars] = m_Scope.top();

		switch (term->kind())
		{
		case Term::Nil:
		{
			m_Scope.pop();
			break;
		}
		case Term::VarCont:
		{
			VarContTerm varCont = term->term<VarContTerm>();

			auto it = boundVars.find(varCont.var);
			if (it != boundVars.end())
			{
				term = varCont.body;
				m_Scope.push(createScope(it->second));
			}
			else
			{
				term = varCont.body;
			}

			break;
		}
		case Term::Abs:
		{
			AbsTerm abs = term->term<AbsTerm>();
			
			boundVars[abs.var] = m_Stacks[abs.loc].back();
			m_Stacks[abs.loc].pop_back();
			term = abs.body;

			break;
		}
		case Term::App:
		{
			AppTerm app = term->term<AppTerm>();
			
			if (app.arg->kind() == Term::VarCont)
			{
				VarContTerm varCont = app.arg->term<VarContTerm>();
				auto it = boundVars.find(varCont.var);
				if (it != boundVars.end())
				{
					m_Stacks[app.loc].push_back(it->second);
					term = app.body;
					break;
				}
			}

			m_Stacks[app.loc].push_back(app.arg);
			term = app.body;

			break;
		}
		}
	}
}

void Machine::printDebug()
{
	// std::cout << std::endl << "--- Binds ----" << std::endl;

	// for (auto it = m_Bound.begin(); it != m_Bound.end(); ++it)
	// {
	// 	const Term *term = it->second;

	// 	switch (term->kind())
	// 	{
	// 	case Term::Nil:
	// 	{
	// 		std::cout << it->first << " -> " << "*" << std::endl;
	// 		break;
	// 	}
	// 	case Term::VarCont:
	// 	{
	// 		VarContTerm varCont = term->term<VarContTerm>();
	// 		std::cout << it->first << " -> " << varCont.var << std::endl;
	// 		break;
	// 	}
	// 	case Term::Abs:
	// 	{
	// 		std::cout << it->first << " -> " << "Abs" << std::endl;
	// 		break;
	// 	}
	// 	case Term::App:
	// 	{
	// 		std::cout << it->first << " -> " << "App" << std::endl;
	// 		break;
	// 	}
	// 	}
	// }

	// std::cout << "--------------" << std::endl;

	std::cout << std::endl << "--- Locations ---" << std::endl;

	for (auto itMap = m_Stacks.begin(); itMap != m_Stacks.end(); ++itMap)
	{
		std::cout << itMap->first.loc << std::endl;
		for (auto itVec = itMap->second.begin(); itVec != itMap->second.end(); ++itVec)
		{
			const Term *term = *itVec;

			switch (term->kind())
			{
			case Term::Nil:
			{
				std::cout << "    " << "*" << std::endl;
				break;
			}
			case Term::VarCont:
			{
				VarContTerm varCont = term->term<VarContTerm>();
				std::cout << "    " << varCont.var.var << std::endl;
				break;
			}
			case Term::Abs:
			{
				std::cout << "    " << "Abs" << std::endl;
				break;
			}
			case Term::App:
			{
				std::cout << "    " << "App" << std::endl;
				break;
			}
			}
		}

		auto itMapCopy = itMap;
		if (!(++itMapCopy == m_Stacks.end()))
		{
			std::cout << std::endl;
		}
	}

	std::cout << "--------------" << std::endl;
}