#include "Machine.hpp"

#include <sstream>

#include "Utils.hpp"

static void machineError(std::string message, const Machine &machine)
{
	std::string stackDebug = machine.getStackDebug();
	std::string callstackDebug = machine.getCallstackDebug();
	// std::string bindDebug = machine.getBindDebug();

	std::cerr << "[Machine Error] ";
	std::cerr << message << std::endl;

	{
		std::stringstream ss(callstackDebug);
		std::string line;

		while (std::getline(ss, line))
		{
			std::cerr << "| " << line << std::endl;
		}
	}

	{
		std::stringstream ss(stackDebug);
		std::string line;

		while (std::getline(ss, line))
		{
			std::cerr << "| " << line << std::endl;
		}
	}

	// {
	// 	std::stringstream ss(bindDebug);
	// 	std::string line;

	// 	while (std::getline(ss, line))
	// 	{
	// 		std::cerr << "| " << line << std::endl;
	// 	}
	// }

	std::exit(1);
}

static std::string locGenerator()
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

	return "loc_" + str;
}

void Machine::execute(const Program &program)
{
	m_VarBindCtx.clear();
	m_LocVarBindCtx.clear();
	while (!m_Frame.empty())
	{
		m_Frame.pop_back();
		m_CallStack.clear();
	}

	if (auto termOpt = program.load("main"))
	{
		m_Frame.push_back(std::make_pair(termOpt.value(), std::make_pair(BoundVars_t(), BoundLocVars_t())));
		m_CallStack.push_back({"main", termOpt.value()});
	}
	else
	{
		machineError("Program has no entry point ('main' is not defined)!", *this);
		return;
	}

	while (!m_Frame.empty())
	{
		// Copy the term and its bound variables because we pop afterwards
		auto [term, bound] = m_Frame.back();
		m_Frame.pop_back();

		auto [boundVars, boundLocVars] = bound;

		if (term->isNil())
		{
			if (!m_CallStack.empty())
			{
				m_CallStack.pop_back();
			}
		}
		else if (term->isVar())
		{
			const VarTerm &var = term->asVar();

			// Push continuation term to frame
			m_Frame.push_back({var.getBody(), {boundVars, boundLocVars}});

			// Find bound term
			auto itBoundVars = boundVars.find(var.getVar());
			if (itBoundVars != boundVars.end())
			{
				// Find binding context for bound term
				auto itVarBindCtx = m_VarBindCtx.find(itBoundVars->second);
				if (itVarBindCtx != m_VarBindCtx.end())
				{
					// Push bound term and its binding context to frame
					m_Frame.push_back({itBoundVars->second, {itVarBindCtx->second, boundLocVars}});
					m_CallStack.push_back({"Binding of '" + var.getVar() + "'", itBoundVars->second});
				}
				else
				{
					machineError("Variable '" + var.getVar() + "' "
						+ "is bound to '" + stringifyTerm(itBoundVars->second) + "' "
						+ "but did not have an environment binding ! "
						+ "Be weary of variable capture !", *this
					);
				}
			}
			else
			{
				if (auto termOpt = program.load(var.getVar()))
				{
					// Push bound term and a new binding context to frame
					m_Frame.push_back({termOpt.value(), {BoundVars_t(), BoundLocVars_t()}});
					m_CallStack.push_back({var.getVar(), termOpt.value()});
				}
				else
				{
					machineError("Variable '" + var.getVar() + "' "
						+ "is not bound to anything !", *this
					);
				}
			}
		}
		else if (term->isAbs())
		{
			const AbsTerm &abs = term->asAbs();
			TermHandle_t toPop;

			auto absActionWithLoc = [&](Loc_t loc) {
				// New stream
				if (loc == k_NewLoc)
				{
					Loc_t newLoc = locGenerator();
					toPop = freshTerm(ValTerm(newLoc));

					m_Stacks[newLoc] = {};
				}
				// Input stream
				else if (loc == k_InputLoc)
				{
					std::string in;
					std::cin >> in;

					Parser parser;
					if (auto termOpt = parser.parseTerm(in))
					{
						toPop = freshTerm(std::move(termOpt.value()));
					}
				}
				// Output stream
				else if (loc == k_OutputLoc)
				{
					machineError("Abstraction is attemping to bind from output location !", *this);
				}
				// Null stream
				else if (loc == k_NullLoc)
				{
					machineError("Abstraction is attemping to bind from null location !", *this);
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
						machineError("Abstraction is attempting to bind from empty stack '"
							+ loc + "' !", *this
						);
					}
				}
			};

			// Reserved stack/stream
			if (auto locOpt = getReservedLocFromId(abs.getLoc()))
			{
				absActionWithLoc(locOpt.value());
			}
			// Generic stack
			else
			{
				auto itBound = boundLocVars.find(abs.getLoc());
				if (itBound != boundLocVars.end())
				{
					absActionWithLoc(itBound->second);
				}
				else
				{
					machineError("Abstraction location '" + abs.getLoc() + "' "
						+ "is not bound to anything !", *this
					);
				}
			}

			// Update global binding context for binding variable
			if (abs.getVar())
			{
				boundVars[abs.getVar().value()] = toPop;
			}

			// Push continuation term to frame
			m_Frame.push_back(std::make_pair(abs.getBody(), std::make_pair(boundVars, boundLocVars)));
		}
		else if (term->isApp())
		{
			const AppTerm &app = term->asApp();
			TermHandle_t toPush = app.getArg();

			// Find and push argument term to stack
			if (app.getArg()->isVar())
			{
				const VarTerm &var = app.getArg()->asVar();

				auto itBound = boundVars.find(var.getVar());
				if (itBound != boundVars.end())
				{
					toPush = itBound->second;
				}
				else if (!program.load(var.getVar()))
				{
					machineError("Application argument '" + var.getVar() + "' "
						+ "is not bound to anything !", *this
					);
				}
			}

			auto appActionWithLoc = [&](Loc_t loc) {
				// New stream
				if (loc == k_NewLoc)
				{
					machineError("Application is attemping to push to new location !", *this);
				}
				// Input stream
				else if (loc == k_InputLoc)
				{
					machineError("Application is attemping to push to input location !", *this);
				}
				// Output stream
				else if (loc == k_OutputLoc)
				{
					// Basic 'cheaty' implementation
					std::cout << stringifyTerm(toPush) << std::endl;
				}
				// Null stream
				else if (loc == k_OutputLoc)
				{
					// Do nothing !
				}
				// Generic stream
				else
				{
					m_Stacks[loc].push_back(toPush);
				}
			};

			// Reserved stack/stream
			if (auto locOpt = getReservedLocFromId(app.getLoc()))
			{
				appActionWithLoc(locOpt.value());
			}
			// Generic stack
			else
			{
				auto itBound = boundLocVars.find(app.getLoc());
				if (itBound != boundLocVars.end())
				{
					appActionWithLoc(itBound->second);
				}
				else
				{
					machineError("Application location '" + app.getLoc() + "' "
						+ "is not bound to anything !", *this
					);
				}
			}

			// Update binding context of argument term
			m_VarBindCtx[toPush] = boundVars;

			// Push continuation term to frame
			m_Frame.push_back(std::make_pair(app.getBody(), std::make_pair(boundVars, boundLocVars)));
		}
		else if (term->isLocAbs())
		{
			const LocAbsTerm &locAbs = term->asLocAbs();
			Loc_t toPop;

			auto absActionWithLoc = [&](Loc_t loc) {
				// New stream
				if (loc == k_NewLoc)
				{
					Loc_t newLoc = locGenerator();
					toPop = newLoc;

					m_Stacks[newLoc] = {};
				}
				// Input stream
				else if (loc == k_InputLoc)
				{
					machineError("Location abstraction is attemping to bind from input location !", *this);
				}
				// Output stream
				else if (loc == k_OutputLoc)
				{
					machineError("Location abstraction is attemping to bind from output location !", *this);
				}
				// Null stream
				else if (loc == k_NullLoc)
				{
					machineError("[Machine Error] Location abstraction is attemping to bind from null location !", *this);
				}
				// Generic stream
				else
				{
					if (!m_Stacks[loc].empty())
					{
						TermHandle_t toPopTerm = m_Stacks[loc].back();
						
						if (toPopTerm->isVal())
						{
							const ValTerm &val = toPopTerm->asVal();

							if (val.isLoc())
							{
								toPop = val.asLoc();
							}
							else
							{
								machineError("Location abstraction is attempting to bind non-location-value'", *this);
							}
						}
						else
						{
							machineError("Location abstraction is attempting to bind non-value'", *this);
						}
						
						m_Stacks[loc].pop_back();
					}
					else
					{
						machineError("Location abstraction is attempting to bind from empty location '"
							+ loc + "' !", *this
						);
					}
				}
			};

			// Reserved stack/stream
			if (auto locOpt = getReservedLocFromId(locAbs.getLoc()))
			{
				absActionWithLoc(locOpt.value());
			}
			// Generic stack
			else
			{
				auto itBound = boundLocVars.find(locAbs.getLoc());
				if (itBound != boundLocVars.end())
				{
					absActionWithLoc(itBound->second);
				}
				else
				{
					machineError("Location abstraction location '" + locAbs.getLoc() + "' "
						+ "is not bound to anything !", *this
					);
				}
			}

			// Update global binding context for binding variable
			if (locAbs.getLocVar())
			{
				boundLocVars[locAbs.getLocVar().value()] = toPop;
			}

			// Push continuation term to frame
			m_Frame.push_back(std::make_pair(locAbs.getBody(), std::make_pair(boundVars, boundLocVars)));
		}
		else if (term->isLocApp())
		{
			const LocAppTerm &locApp = term->asLocApp();
			TermHandle_t toPush;

			// Find and push argument term to stack
			auto it = boundLocVars.find(locApp.getArg());
			if (it != boundLocVars.end())
			{
				toPush = freshTerm(ValTerm(it->second));;
			}
			else if (locApp.getArg() == k_LambdaLoc)
			{
				toPush = freshTerm(ValTerm(k_LambdaLoc));
			}
			else if (locApp.getArg() == k_NewLoc)
			{
				toPush = freshTerm(ValTerm(k_NewLoc));
			}
			else if (locApp.getArg() == k_InputLoc)
			{
				toPush = freshTerm(ValTerm(k_InputLoc));
			}
			else if (locApp.getArg() == k_OutputLoc)
			{
				toPush = freshTerm(ValTerm(k_OutputLoc));
			}
			else if (locApp.getArg() == k_NullLoc)
			{
				toPush = freshTerm(ValTerm(k_NullLoc));
			}
			else
			{
				machineError("Location application argument '" + locApp.getArg() + "' "
					+ "is not bound to anything !", *this
				);
			}

			auto appActionWithLoc = [&](Loc_t loc) {
				// New stream
				if (loc == k_NewLoc)
				{
					machineError("Location application is attemping to push to 'new' location ! ", *this);
				}
				// Input stream
				else if (loc == k_InputLoc)
				{
					machineError("Location application is attemping to push to 'input' location ! ", *this);
				}
				// Output stream
				else if (loc == k_OutputLoc)
				{
					// Basic 'cheaty' implementation
					std::cout << stringifyTerm(toPush) << std::endl;
				}
				// Null stream
				else if (loc == k_OutputLoc)
				{
					// Do nothing !
				}
				// Generic stream
				else
				{
					m_Stacks[loc].push_back(toPush);
				}
			};

			// Reserved stack/stream
			if (auto locOpt = getReservedLocFromId(locApp.getLoc()))
			{
				appActionWithLoc(locOpt.value());
			}
			// Generic stack
			else
			{
				auto itBound = boundLocVars.find(locApp.getLoc());
				if (itBound != boundLocVars.end())
				{
					appActionWithLoc(itBound->second);
				}
				else
				{
					machineError("Location application location '" + locApp.getLoc() + "' "
						+ "is not bound to anything !", *this
					);
				}
			}

			// Update binding context of argument term
			m_LocVarBindCtx[toPush] = boundLocVars;

			// Push continuation term to frame
			m_Frame.push_back(std::make_pair(locApp.getBody(), std::make_pair(boundVars, boundLocVars)));
		}
		else if (term->isVal())
		{
			machineError("Value term is being executed by machine ! Perhaps something wasn't pushed to the stack !", *this);
		}
		else if (term->isBinOp())
		{
			const BinOpTerm &binOp = term->asBinOp();
			TermHandle_t toPush;
			TermHandle_t toPop1;
			TermHandle_t toPop2;

			if (!m_Stacks[Loc_t(k_LambdaLoc)].empty())
			{
				toPop1 = m_Stacks[k_LambdaLoc].back();
				m_Stacks[Loc_t(k_LambdaLoc)].pop_back();
			}
			else
			{
				machineError("Binary operation is attempting to pop from empty stack !", *this);
			}

			if (!m_Stacks[Loc_t(k_LambdaLoc)].empty())
			{
				toPop2 = m_Stacks[k_LambdaLoc].back();
				m_Stacks[Loc_t(k_LambdaLoc)].pop_back();
			}
			else
			{
				machineError("Binary operation is attempting to pop from empty stack !", *this);
			}

			if (toPop1->isVal())
			{
				if (toPop2->isVal())
				{
					const ValTerm &val1 = toPop1->asVal();
					const ValTerm &val2 = toPop2->asVal();
					auto prim1 = val1.asPrim();
					auto prim2 = val2.asPrim();
					
					if (binOp.isOp(BinOpTerm::Plus))
					{
						toPush = freshTerm(ValTerm(prim2 + prim1));
					}
					else if (binOp.isOp(BinOpTerm::Minus))
					{
						toPush = freshTerm(ValTerm(prim2 - prim1));
					}
				}
				else
				{
					machineError("Binary operation is attempting to use a non-value as second popped term !", *this);
				}
			}
			else
			{
				machineError("Binary operation is attempting to use a non-value as first popped value !", *this);
			}

			// Push to the stack
			m_Stacks[k_LambdaLoc].push_back(toPush);

			// Update binding context of argument term
			m_VarBindCtx[toPush] = boundVars;

			// Push continuation term to frame
			m_Frame.push_back(std::make_pair(binOp.getBody(), std::make_pair(boundVars, boundLocVars)));
		}
		else if (term->isPrimCases())
		{
			const CasesTerm<Prim_t> &cases = term->asPrimCases();
			TermHandle_t toPop;

			// Push continuation term to frame
			m_Frame.push_back(std::make_pair(cases.getBody(), std::make_pair(boundVars, boundLocVars)));

			if (!m_Stacks[Loc_t(k_LambdaLoc)].empty())
			{
				toPop = m_Stacks[k_LambdaLoc].back();
				m_Stacks[Loc_t(k_LambdaLoc)].pop_back();
			}
			else
			{
				machineError("Cases is attempting to match from empty stack !", *this);
			}

			if (toPop->isVal())
			{
				const ValTerm &val = toPop->asVal();

				if (val.isPrim())
				{
					auto itCase = cases.find(val.asPrim());
					if (itCase != cases.end())
					{
						// Push case term and the global binding context to frame
						m_Frame.push_back(std::make_pair(itCase->second, std::make_pair(boundVars, boundLocVars)));
						m_CallStack.push_back({"Case '" + std::to_string(val.asPrim()) + "'", itCase->second});
					}
					else
					{
						// Push case term and the global binding context to frame
						m_Frame.push_back({cases.getOtherwise(), {boundVars, boundLocVars}});
						m_CallStack.push_back({"Case 'otherwise'", cases.getOtherwise()});
					}
				}
				else if (val.isLoc())
				{
					machineError("Cases expect a primitive value instead of a location !", *this);
				}
			}
			else
			{
				machineError("Cases is attemping to match a non-value !", *this);
			}
		}
		else if (term->isLocCases())
		{
			const CasesTerm<Loc_t> &cases = term->asLocCases();
			TermHandle_t toPop;

			// Push continuation term to frame
			m_Frame.push_back(std::make_pair(cases.getBody(), std::make_pair(boundVars, boundLocVars)));

			if (!m_Stacks[Loc_t(k_LambdaLoc)].empty())
			{
				toPop = m_Stacks[k_LambdaLoc].back();
				m_Stacks[Loc_t(k_LambdaLoc)].pop_back();
			}
			else
			{
				machineError("Cases is attempting to match from empty stack !", *this);
			}

			if (toPop->isVal())
			{
				const ValTerm &val = toPop->asVal();

				if (val.isPrim())
				{
					machineError("Cases expect a location value instead of a primitive !", *this);
				}
				else if (val.isLoc())
				{
					auto itCase = cases.find(val.asLoc());
					if (itCase != cases.end())
					{
						// Push case term and the global binding context to frame
						m_Frame.push_back(std::make_pair(itCase->second, std::make_pair(boundVars, boundLocVars)));
						m_CallStack.push_back({"Case '" + val.asLoc() + "'", itCase->second});
					}
					else
					{
						// Push case term and the global binding context to frame
						m_Frame.push_back({cases.getOtherwise(), {boundVars, boundLocVars}});
						m_CallStack.push_back({"Case 'otherwise'", cases.getOtherwise()});
					}
				}
			}
			else
			{
				machineError("Cases is attemping to match a non-value !", *this);
			}
		}
	}
}

TermHandle_t Machine::freshTerm(Term &&term)
{
	m_FreshTerms.push_back(newTerm(std::move(term)));	
	return m_FreshTerms.back();
}

std::string Machine::getStackDebug() const
{
	std::stringstream ss;

	ss << "---- Stacks ----" << '\n';

	for (auto itStacks = m_Stacks.begin(); itStacks != m_Stacks.end(); ++itStacks)
	{
		if (auto idOpt = getIdFromReservedLoc(itStacks->first))
		{
			ss << "  -- (Reserved) Location " << idOpt.value() << '\n';
		}
		else
		{
			ss << "  -- Location " << itStacks->first << '\n';
		}

		for (auto itStack = itStacks->second.rbegin(); itStack != itStacks->second.rend(); ++itStack)
		{
			ss << "    " << stringifyTerm((*itStack)) << '\n';
		}

		auto itStacksCopy = itStacks;
		if (!(++itStacksCopy == m_Stacks.end()))
		{
			ss << '\n';
		}
	}

	ss << "--------------------";

	return ss.str();
}

std::string Machine::getCallstackDebug() const
{
	std::stringstream ss;

	ss << "---- Call Stack ----" << '\n';

	for (auto itFrame = m_CallStack.rbegin(); itFrame != m_CallStack.rend(); ++itFrame)
	{
		ss << std::string(m_CallStack.size() - (m_CallStack.rend() - itFrame), ' ');
		ss << "> " << itFrame->first << " => " << stringifyTerm(itFrame->second) << '\n';
	}

	ss << "---------------";

	return ss.str();
}

std::string Machine::getBindDebug() const
{
	std::stringstream ss;

	ss << "--- Bind Context ---" << '\n';

	for (auto itBindCtx = m_VarBindCtx.begin(); itBindCtx != m_VarBindCtx.end(); ++itBindCtx)
	{
		ss << "  -- Binds for term " << stringifyTerm(itBindCtx->first) << '\n';

		for (auto itBinds = itBindCtx->second.begin(); itBinds != itBindCtx->second.end(); ++itBinds)
		{
			ss << "    " << (*itBinds).first << " --> " << stringifyTerm(itBinds->second) << '\n';
		}

		auto itBindCtxCopy = itBindCtx;
		if (!(++itBindCtxCopy == m_VarBindCtx.end()))
		{
			ss << '\n';
		}
	}

	ss << "--------------------";

	return ss.str();
}