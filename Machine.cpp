#include "Machine.hpp"

#include "Utils.hpp"

Machine::Machine(const FuncDefs_t *funcs)
	: m_Funcs(funcs)
{
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
		case Term::Val:
		{
			std::cout << "[WARN] Value term is being executed by machine ! "
				<< "Perhaps you forgot to push something to the stack !"
				<< std::endl;
			
			break;
		}
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
					std::cout << "[WARN] Variable '" << varCont.var << "' "
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
				auto it = m_Funcs->find(varCont.var);
				if (it != m_Funcs->end())
				{
					// Push bound term and a new binding context to frame
					m_Frame.push(std::make_pair(&it->second, BoundVars_t()));
				}
				else
				{
					std::cout << "[WARN] Variable '" << varCont.var << "' "
						<< "is not bound to anything !"
						<< std::endl;
				}
			}

			break;
		}
		case Term::Abs:
		{
			const AbsTerm &abs = term->asAbs();

			const Term *toPop;

			// New stream
			if (abs.loc == k_NewLoc)
			{
				static Val_t s_NewLoc = k_FirstNewLoc;

				// Need some way to generate 'new' terms without leaking memory :/
				Term *term = new Term();
				*term = ValTerm(s_NewLoc);
				toPop = term;

				s_NewLoc++;
			}
			// Input stream
			else if (abs.loc == k_InputLoc)
			{
				std::string in;
				std::getline(std::cin, in);

				// Need some way to generate 'new' terms without leaking memory :/
				Parser parser;
				Term *term = new Term();
				*term = std::move(parser.parseTerm(in));
				toPop = term;
			}
			// Output stream
			else if (abs.loc == k_OutputLoc)
			{
				std::cout << "[WARN] Abstraction is attemping to bind from output location ! "
					<< std::endl;
				
				std::exit(1);
			}
			// Generic stream
			else
			{
				toPop = m_Stacks[abs.loc].back();
				m_Stacks[abs.loc].pop_back();
			}

			// Update global binding context for binding variable
			bound[abs.var] = toPop;

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

			// New stream
			if (app.loc == k_NewLoc)
			{
				std::cout << "[WARN] Application is attemping to push to new location ! "
					<< std::endl;

				std::exit(1);
			}
			// Input stream
			else if (app.loc == k_InputLoc)
			{
				std::cout << "[WARN] Application is attemping to push to input location ! "
					<< std::endl;

				std::exit(1);
			}
			// Output stream
			else if (app.loc == k_OutputLoc)
			{
				// Basic 'cheaty' implementation
				std::cout << stringifyTerm(*toPush) << std::endl;
			}
			// Generic stream
			else
			{
				m_Stacks[app.loc].push_back(toPush);
			}

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
	std::cout << "---- Stacks ----" << std::endl;

	for (auto itStacks = m_Stacks.begin(); itStacks != m_Stacks.end(); ++itStacks)
	{
		std::cout << "  -- " << itStacks->first << std::endl;

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

	std::cout << "--------------------" << std::endl;
	std::cout << "--- Bind Context ---" << std::endl;

	for (auto itBindCtx = m_BindCtx.begin(); itBindCtx != m_BindCtx.end(); ++itBindCtx)
	{
		std::cout << "  -- Binds for term " << stringifyTerm(*(*itBindCtx).first) << std::endl;

		for (auto itBinds = itBindCtx->second.begin(); itBinds != itBindCtx->second.end(); ++itBinds)
		{
			std::cout << "    " << (*itBinds).first << " --> " << stringifyTerm(*(*itBinds).second) << std::endl;
		}

		auto itBindCtxCopy = itBindCtx;
		if (!(++itBindCtxCopy == m_BindCtx.end()))
		{
			std::cout << std::endl;
		}
	}

	std::cout << "--------------------" << std::endl;
}