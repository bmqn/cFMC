#include "Machine.hpp"

#include "Utils.hpp"

Machine::Machine(const FuncDefs_t *funcs)
	: m_Funcs(funcs)
{
	static Term zero = VarContTerm("0");
	static Term two = VarContTerm("2");
	static Term seven = VarContTerm("7");
	static Term nine = VarContTerm("9");

	m_Stacks[k_DefaultLoc].push_back(&zero);
	m_Stacks[k_RandomLoc].push_back(&two);
	m_Stacks[k_RandomLoc].push_back(&seven);
	m_Stacks[k_RandomLoc].push_back(&nine);
}

void Machine::execute()
{
	m_BindCtx.clear();
	while (!m_Frame.empty())
	{
		m_Frame.pop();
	}

	auto it = m_Funcs->find("main");
	if (it == m_Funcs->end())
	{
		std::cerr << "[Machine Error] Program could not load entry point ('main' is not defined)!" << std::endl;
		return;
	}
	m_Frame.push(std::make_pair(&it->second, BoundVars_t()));

	while (!m_Frame.empty())
	{
		// Copy the term and its bound variables because we pop afterwards
		auto [term, bound] = m_Frame.top();
		m_Frame.pop();

		switch (term->kind())
		{
		case Term::Nil:
		{
			break;
		}
		case Term::VarCont:
		{
			const VarContTerm &varCont = term->asVarCont();

			// Push continuation term to frame
			m_Frame.push(std::make_pair(varCont.body.get(), bound));

			// Find bound term
			auto itBound = bound.find(varCont.var);
			if (itBound != bound.end())
			{
				// Find binding context for bound term
				auto ItEnv = m_BindCtx.find(itBound->second);
				if (ItEnv != m_BindCtx.end())
				{
					// Push bound term and its binding context to frame
					m_Frame.push(std::make_pair(itBound->second, ItEnv->second));
				}
				else
				{
					std::cout << "[INFO] Variable '" << varCont.var.var << "' "
						<< "is bound to '" << stringifyTerm(*itBound->second) << "' "
						<< "but did not have an environment binding ! "
						<< "Be weary of variable capture !"
						<< std::endl;

					// Push bound term and the global binding context to frame
					m_Frame.push(std::make_pair(itBound->second, bound));
				}
			}
			else
			{
				auto it = m_Funcs->find(varCont.var.var);
				if (it != m_Funcs->end())
				{
					// Push bound term and a new binding context to frame
					m_Frame.push(std::make_pair(&it->second, BoundVars_t()));
				}
				else
				{
					std::cout << "[INFO] Variable '" << varCont.var.var << "' "
						<< "is not bound to anything !"
						<< std::endl;
				}
			}

			break;
		}
		case Term::Abs:
		{
			const AbsTerm &abs = term->asAbs();

			// Update global binding context for binding variable
			bound[abs.var] = m_Stacks[abs.loc].back();
			m_Stacks[abs.loc].pop_back();

			// Push continuation term to frame
			m_Frame.push(std::make_pair(abs.body.get(), bound));

			break;
		}
		case Term::App:
		{
			const AppTerm &app = term->asApp();

			// Find and push argument term to stack
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

			// Update binding context of argument term
			m_BindCtx[toPush] = bound;

			// Push continuation term to frame
			m_Frame.push(std::make_pair(app.body.get(), bound));

			break;
		}
		}
	}
}

void Machine::printDebug()
{
	std::cout << "--- Stacks ---" << std::endl;

	for (auto itStacks = m_Stacks.begin(); itStacks != m_Stacks.end(); ++itStacks)
	{
		std::cout << itStacks->first.loc << std::endl;

		for (auto itStack = itStacks->second.rbegin(); itStack != itStacks->second.rend(); ++itStack)
		{
			std::cout << "    " << stringifyTerm(*(*itStack)) << std::endl;
		}

		auto itStacksCopy = itStacks;
		if (!(++itStacksCopy == m_Stacks.end()))
		{
			std::cout << std::endl;
		}
	}

	std::cout << "------------------" << std::endl;
}