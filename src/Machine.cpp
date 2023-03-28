#include "Machine.hpp"

#include "Utils.hpp"

static std::string varGenerator()
{
	constexpr const char *k_Src = "xyzwv";
	static int ptrs[] = {0, 0, 0, 0, 0};

	std::string str;
	str += k_Src[ptrs[0] % 5];
	str += k_Src[ptrs[1] % 5];
	str += k_Src[ptrs[2] % 5];
	str += k_Src[ptrs[3] % 5];
	str += k_Src[ptrs[4] % 5];

	if (ptrs[0] < 5 * 5 * 5 * 5 * 5 - 1)
	{
		ptrs[0] = (ptrs[0] + 1); 
		ptrs[1] = (ptrs[1] + (ptrs[0] % 5 == 0));
		ptrs[2] = (ptrs[2] + (ptrs[0] % 25 == 0));
		ptrs[3] = (ptrs[3] + (ptrs[0] % 125 == 0));
		ptrs[4] = (ptrs[4] + (ptrs[0] % 725 == 0));
	}
	else
	{
		std::cerr << "Ran out of generator variables !" << std::endl;
	}

	return str;
}

Machine::Machine(const FuncDefs_t *funcs)
	: m_Funcs(funcs)
{
}

void Machine::execute()
{
	m_VarBindCtx.clear();
	m_LocVarBindCtx.clear();
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
	m_Frame.push(std::make_pair(&it->second, std::make_pair(BoundVars_t(), BoundLocVars_t())));

	while (!m_Frame.empty())
	{
		// Copy the term and its bound variables because we pop afterwards
		auto [term, bound] = m_Frame.top();
		auto [boundVars, boundLocVars] = bound;
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
			m_Frame.push(std::make_pair(varCont.getBody(), std::make_pair(boundVars, boundLocVars)));

			// Find bound term
			auto itBound = boundVars.find(varCont.getVar());
			if (itBound != boundVars.end())
			{
				// Find binding context for bound term
				auto ItEnv = m_VarBindCtx.find(itBound->second);
				if (ItEnv != m_VarBindCtx.end())
				{
					// Push bound term and its binding context to frame
					m_Frame.push(std::make_pair(itBound->second, std::make_pair(ItEnv->second, boundLocVars)));
				}
				else
				{
					std::cerr << "[Machine Error] Variable '" << varCont.getVar() << "' "
						<< "is bound to '" << stringifyTerm(*itBound->second) << "' "
						<< "but did not have an environment binding ! "
						<< "Be weary of variable capture !"
						<< std::endl;

					// Push bound term and the global binding context to frame
					m_Frame.push(std::make_pair(itBound->second, std::make_pair(boundVars, boundLocVars)));
				}
			}
			else
			{
				auto itFunc = m_Funcs->find(varCont.getVar());
				if (itFunc != m_Funcs->end())
				{
					// Push bound term and a new binding context to frame
					m_Frame.push(std::make_pair(&itFunc->second, std::make_pair(BoundVars_t(), BoundLocVars_t())));
				}
				else
				{
					std::cerr << "[Machine Error] Variable '" << varCont.getVar() << "' "
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

			auto absActionWithLoc = [&](Loc_t loc) {
				// Default stack
				if (loc == k_DefaultLoc)
				{
					if (!m_Stacks[Loc_t(k_DefaultLoc)].empty())
					{
						toPop = m_Stacks[Loc_t(k_DefaultLoc)].back();
						m_Stacks[Loc_t(k_DefaultLoc)].pop_back();
					}
					else
					{
						std::cerr << "[Machine Error] Abstraction is attempting to bind from empty stack !" << std::endl;
						std::exit(1);
					}
				}
				// New stream
				else if (loc == k_NewLoc)
				{
					// Need some way to generate 'new' terms without leaking memory :/
					Loc_t newLoc = varGenerator();
					Term *term = new Term();
					*term = ValTerm(newLoc);
					toPop = term;

					m_Stacks[newLoc] = {};
				}
				// Input stream
				else if (loc == k_InputLoc)
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
				else if (loc == k_OutputLoc)
				{
					std::cerr << "[Machine Error] Abstraction is attemping to bind from output location ! "
						<< std::endl;
					std::exit(1);
				}
				// Generic stack
				else
				{
					if (!m_Stacks[loc].empty())
					{
						toPop = m_Stacks[loc].back();
						m_Stacks[loc].pop_back();
					}
					else
					{
						std::cerr << "[Machine Error] Abstraction is attempting to bind from empty stack '"
							<< loc << "' !"
							<< std::endl;
						std::exit(1);
					}
				}
			};

			// Reserved stack/stream
			if (auto locOpt = getReservedLocFromId(abs.loc))
			{
				absActionWithLoc(locOpt.value());
			}
			// Generic stack
			else
			{
				auto itBound = boundLocVars.find(abs.loc);
				if (itBound != boundLocVars.end())
				{
					absActionWithLoc(itBound->second);
				}
				else
				{
					std::cerr << "[Machine Error] Abstraction location '" << abs.loc << "' "
						<< "is not bound to anything !"
						<< std::endl;
					std::exit(1);
				}
			}

			// Update global binding context for binding variable
			if (abs.var)
			{
				boundVars[abs.var.value()] = toPop;
			}

			// Push continuation term to frame
			m_Frame.push(std::make_pair(abs.body.get(), std::make_pair(boundVars, boundLocVars)));

			break;
		}
		case Term::App:
		{
			const AppTerm &app = term->asApp();
			const Term *toPush = app.arg.get();

			// Find and push argument term to stack
			if (app.arg->kind() == Term::VarCont)
			{
				const VarContTerm &varCont = app.arg->asVarCont();

				auto itBound = boundVars.find(varCont.getVar());
				if (itBound != boundVars.end())
				{
					toPush = itBound->second;
				}
				// Default stack
				else if (varCont.is(Var_t(k_DefaultLoc)))
				{
					// Need some way to generate 'new' terms without leaking memory :/
					Term *term = new Term();
					*term = ValTerm(Loc_t(k_DefaultLoc));
					toPush = term;
				}
				else if (varCont.is(Var_t(k_NewLoc)))
				{
					// Need some way to generate 'new' terms without leaking memory :/
					Term *term = new Term();
					*term = ValTerm(Loc_t(k_NewLoc));
					toPush = term;
				}
				else if (varCont.is(Var_t(k_InputLoc)))
				{
					// Need some way to generate 'new' terms without leaking memory :/
					Term *term = new Term();
					*term = ValTerm(Loc_t(k_InputLoc));
					toPush = term;
				}
				else if (varCont.is(Var_t(k_OutputLoc)))
				{
					// Need some way to generate 'new' terms without leaking memory :/
					Term *term = new Term();
					*term = ValTerm(Loc_t(k_OutputLoc));
					toPush = term;
				}
				else
				{
					auto itFunc = m_Funcs->find(varCont.getVar());
					if (itFunc == m_Funcs->end())
					{
						std::cerr << "[Machine Error] Application argument '" << varCont.getVar() << "' "
							<< "is not bound to anything !"
							<< std::endl;
						std::exit(1);
					}
				}
			}

			auto appActionWithLoc = [&](Loc_t loc) {
				// Default stack
				if (loc == k_DefaultLoc)
				{
					m_Stacks[loc].push_back(toPush);
				}
				// New stream
				else if (loc == k_NewLoc)
				{
					std::cerr << "[Machine Error] Application is attemping to push to new location ! "
						<< std::endl;
					std::exit(1);
				}
				// Input stream
				else if (loc == k_InputLoc)
				{
					std::cerr << "[Machine Error] Application is attemping to push to input location ! "
						<< std::endl;
					std::exit(1);
				}
				// Output stream
				else if (loc == k_OutputLoc)
				{
					// Basic 'cheaty' implementation
					std::cout << stringifyTerm(*toPush) << std::endl;
				}
				// Generic stream
				else
				{
					m_Stacks[loc].push_back(toPush);
				}
			};

			// Reserved stack/stream
			if (auto locOpt = getReservedLocFromId(app.loc))
			{
				appActionWithLoc(locOpt.value());
			}
			// Generic stack
			else
			{
				auto itBound = boundLocVars.find(app.loc);
				if (itBound != boundLocVars.end())
				{
					appActionWithLoc(itBound->second);
				}
				else
				{
					std::cerr << "[Machine Error] Application location '" << app.loc << "' "
						<< "is not bound to anything !"
						<< std::endl;
					std::exit(1);
				}
			}

			// Update binding context of argument term
			m_VarBindCtx[toPush] = boundVars;

			// Push continuation term to frame
			m_Frame.push(std::make_pair(app.body.get(), std::make_pair(boundVars, boundLocVars)));

			break;
		}
		case Term::LocAbs:
		{
			const LocAbsTerm &locAbs = term->asLocAbs();
			Loc_t toPop;

			auto absActionWithLoc = [&](Loc_t loc) {
				// New stream
				if (loc == k_NewLoc)
				{
					Loc_t newLoc = varGenerator();
					toPop = newLoc;

					m_Stacks[newLoc] = {};
				}
				// Generic stream
				else
				{
					if (!m_Stacks[loc].empty())
					{
						const Term *toPopTerm = m_Stacks[loc].back();
						
						if (toPopTerm->kind() == Term::Val)
						{
							const ValTerm &val = toPopTerm->asVal();

							if (val.kind() == ValTerm::Loc)
							{
								toPop = val.getLoc();
							}
							else
							{
								std::cerr << "[Machine Error] Location abstraction is attempting to bind non-location-value'"
									<< std::endl;
								std::exit(1);
							}
						}
						else
						{
							std::cerr << "[Machine Error] Location abstraction is attempting to bind non-value'"
								<< std::endl;
							std::exit(1);
						}
						
						m_Stacks[loc].pop_back();
					}
					else
					{
						std::cerr << "[Machine Error] Location abstraction is attempting to bind from empty location '"
							<< loc << "' !"
							<< std::endl;
						std::exit(1);
					}
				}
			};

			// Reserved stack/stream
			if (auto locOpt = getReservedLocFromId(locAbs.loc))
			{
				absActionWithLoc(locOpt.value());
			}
			// Generic stack
			else
			{
				auto itBound = boundLocVars.find(locAbs.loc);
				if (itBound != boundLocVars.end())
				{
					absActionWithLoc(itBound->second);
				}
				else
				{
					std::cerr << "[Machine Error] Location abstraction location '" << locAbs.loc << "' "
						<< "is not bound to anything !"
						<< std::endl;
					std::exit(1);
				}
			}

			// Update global binding context for binding variable
			if (locAbs.var)
			{
				boundLocVars[locAbs.var.value()] = toPop;
			}

			// Push continuation term to frame
			m_Frame.push(std::make_pair(locAbs.body.get(), std::make_pair(boundVars, boundLocVars)));

			break;
		}
		case Term::LocApp:
		{
			const LocAppTerm &locApp = term->asLocApp();
			const Term *toPush;

			// Find and push argument term to stack
			auto it = boundLocVars.find(locApp.arg);
			if (it != boundLocVars.end())
			{
				Term *term = new Term();
				*term = ValTerm(Loc_t(it->second));
				toPush = term;
			}
			// Default stack
			else if (locApp.arg == k_DefaultLoc)
			{
				// Need some way to generate 'new' terms without leaking memory :/
				Term *term = new Term();
				*term = ValTerm(Loc_t(k_DefaultLoc));
				toPush = term;
			}
			else if (locApp.arg == k_NewLoc)
			{
				// Need some way to generate 'new' terms without leaking memory :/
				Term *term = new Term();
				*term = ValTerm(Loc_t(k_NewLoc));
				toPush = term;
			}
			else if (locApp.arg == k_InputLoc)
			{
				// Need some way to generate 'new' terms without leaking memory :/
				Term *term = new Term();
				*term = ValTerm(Loc_t(k_InputLoc));
				toPush = term;
			}
			else if (locApp.arg == k_OutputLoc)
			{
				// Need some way to generate 'new' terms without leaking memory :/
				Term *term = new Term();
				*term = ValTerm(Loc_t(k_OutputLoc));
				toPush = term;
			}
			else
			{
				std::cerr << "[Machine Error] Location application argument '" << locApp.arg << "' "
					<< "is not bound to anything !"
					<< std::endl;
				std::exit(1);
			}

			auto appActionWithLoc = [&](Var_t loc) {
				// Default stack
				if (loc == k_DefaultLoc)
				{
					m_Stacks[loc].push_back(toPush);
				}
				// New stream
				else if (loc == k_NewLoc)
				{
					std::cerr << "[Machine Error] Location application is attemping to push to new location ! "
						<< std::endl;
					std::exit(1);
				}
				// Input stream
				else if (loc == k_InputLoc)
				{
					std::cerr << "[Machine Error] Location application is attemping to push to input location ! "
						<< std::endl;
					std::exit(1);
				}
				// Output stream
				else if (loc == k_OutputLoc)
				{
					// Basic 'cheaty' implementation
					std::cout << stringifyTerm(*toPush) << std::endl;
				}
				// Generic stream
				else
				{
					m_Stacks[loc].push_back(toPush);
				}
			};

			// Reserved stack/stream
			if (auto locOpt = getReservedLocFromId(locApp.loc))
			{
				appActionWithLoc(locOpt.value());
			}
			// Generic stack
			else
			{
				auto itBound = boundLocVars.find(locApp.loc);
				if (itBound != boundLocVars.end())
				{
					appActionWithLoc(itBound->second);
				}
				else
				{
					std::cerr << "[Machine Error] Location application location '" << locApp.loc << "' "
						<< "is not bound to anything !"
						<< std::endl;
					std::exit(1);
				}
			}

			// Update binding context of argument term
			m_LocVarBindCtx[toPush] = boundLocVars;

			// Push continuation term to frame
			m_Frame.push(std::make_pair(locApp.body.get(), std::make_pair(boundVars, boundLocVars)));

			break;
		}
		case Term::Val:
		{
			std::cerr << "[Machine Error] Value term is being executed by machine ! "
				<< "Perhaps you forgot to push something to the stack !"
				<< std::endl;
			
			break;
		}
		case Term::Cases:
		{
			const CasesTerm &cases = term->asCases();
			const Term *toPop;

			// Push continuation term to frame
			m_Frame.push(std::make_pair(cases.body.get(), std::make_pair(boundVars, boundLocVars)));

			if (!m_Stacks[Loc_t(k_DefaultLoc)].empty())
			{
				toPop = m_Stacks[Var_t(k_DefaultLoc)].back();
				m_Stacks[Loc_t(k_DefaultLoc)].pop_back();
			}
			else
			{
				std::cerr << "[Machine Error] Cases is attempting to match from empty stack !" << std::endl;
				std::exit(1);
			}

			if (toPop->kind() == Term::Val)
			{
				const ValTerm &val = toPop->asVal();

				if (val.kind() == ValTerm::Prim)
				{
					std::cerr << "[Machine Error] Cases are not implemented for primitives !"
						<< std::endl;
					std::exit(1);
				}
				else if (val.kind() == ValTerm::Loc)
				{
					Loc_t loc = val.getLoc();

					auto itCase = cases.cases.find(loc);
					if (itCase != cases.cases.end())
					{
						// Push case term and the global binding context to frame
						m_Frame.push(std::make_pair(itCase->second.get(), std::make_pair(boundVars, boundLocVars)));
					}
					else
					{
						itCase = cases.cases.find("otherwise");
						if (itCase != cases.cases.end())
						{
							// Push case term and the global binding context to frame
							m_Frame.push(std::make_pair(itCase->second.get(), std::make_pair(boundVars, boundLocVars)));
						}
						else
						{
							std::cerr << "[Machine Error] Cases could not match value and did not have an 'otherwise' pattern !"
										<< std::endl;
							std::exit(1);
						}
					}
				}
			}
			else
			{
				std::cerr << "[Machine Error] Cases is attemping to match a non-value !"
							<< std::endl;
				std::exit(1);
			}

			break;
		}
		}
	}
}

void Machine::printDebug()
{
	std::cerr << "---- Stacks ----" << std::endl;

	for (auto itStacks = m_Stacks.begin(); itStacks != m_Stacks.end(); ++itStacks)
	{
		if (auto idOpt = getIdFromReservedLoc(itStacks->first))
		{
			std::cerr << "  -- Location (Reserved) " << idOpt.value() << std::endl;
		}
		else
		{
			std::cerr << "  -- Location " << itStacks->first << std::endl;
		}

		for (auto itStack = itStacks->second.rbegin(); itStack != itStacks->second.rend(); ++itStack)
		{
			std::cerr << "    " << stringifyTerm(*(*itStack)) << std::endl;
		}

		auto itStacksCopy = itStacks;
		if (!(++itStacksCopy == m_Stacks.end()))
		{
			std::cerr << std::endl;
		}
	}

	std::cerr << "--------------------" << std::endl;
	std::cerr << "--- Bind Context ---" << std::endl;

	for (auto itBindCtx = m_VarBindCtx.begin(); itBindCtx != m_VarBindCtx.end(); ++itBindCtx)
	{
		std::cerr << "  -- Binds for term " << stringifyTerm(*(*itBindCtx).first) << std::endl;

		for (auto itBinds = itBindCtx->second.begin(); itBinds != itBindCtx->second.end(); ++itBinds)
		{
			std::cerr << "    " << (*itBinds).first << " --> " << stringifyTerm(*(*itBinds).second) << std::endl;
		}

		auto itBindCtxCopy = itBindCtx;
		if (!(++itBindCtxCopy == m_VarBindCtx.end()))
		{
			std::cerr << std::endl;
		}
	}

	std::cerr << "--------------------" << std::endl;
}