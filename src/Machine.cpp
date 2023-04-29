#include "Machine.hpp"

#include <sstream>

#include "Utils.hpp"

static void machineError(std::string message, const Machine &machine)
{
	std::string stackDebug = machine.getStackDebug();
	std::string callstackDebug = machine.getCallstackDebug();

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
	m_Memory.clear();
	m_Control.clear();

	if (auto termOpt = program.load("main"))
	{
		m_Control.push_back(
			std::make_pair(Env_t{}, termOpt.value())
		);

		m_CallStack.push_back({"main", termOpt.value()});
	}
	else
	{
		machineError("Program has no entry point ('main' is not defined)!", *this);
		return;
	}

	while (!m_Control.empty())
	{
		// Get the next environment and term
		Closure_t closure = m_Control.back();
		Env_t env = Env_t(closure.first);
		TermHandle_t term = closure.second;
		m_Control.pop_back();

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

			// Push continuation term
			m_Control.push_back(std::make_pair(env, var.getBody()));

			// We found term in our environment
			auto itEnv = env.first.find(var.getVar());
			if (itEnv != env.first.end())
			{
				// Push bound term
				auto closurePtr = itEnv->second;
				Closure_t *closure = reinterpret_cast<Closure_t *>(closurePtr.get());
				m_Control.push_back(*closure);
				m_CallStack.push_back({"Binding of '" + var.getVar() + "'", closure->second});
			}
			// We found term in our program functions
			else if (auto termOpt = program.load(var.getVar()))
			{
				// Push program function
				m_Control.push_back(std::make_pair(Env_t{}, termOpt.value()));
				m_CallStack.push_back({var.getVar(), closure.second});
			}
			// We didn't find our term anywhere.. error !
			else
			{
				machineError("Variable '" + var.getVar() + "' "
					+ "is not bound to anything !", *this);
			}
		}
		else if (term->isApp())
		{
			const AppTerm &app = term->asApp();

			m_Control.push_back(std::make_pair(env, app.getBody()));

			auto appActionWithLoc = [&](Loc_t loc) {
				// New stream
				if (loc == k_NewLoc)
				{
					machineError("Application cannot push to 'new' location !", *this);
				}
				// Input stream
				else if (loc == k_InputLoc)
				{
					machineError("Application cannot push to 'input' location !", *this);
				}
				// Output stream
				else if (loc == k_OutputLoc)
				{
					std::cout << stringifyClosure(std::make_pair(env, app.getArg())) << std::endl;
				}
				// Null stream
				else if (loc == k_NullLoc)
				{}
				// Generic stack
				else
				{
					// If we have variables that are bound to values, then push them directly
					// instead of as variables. This makes it much easier to deal with values
					// in binary operations and cases etc.

					bool hasPushedAsValue = false;
					
					if (app.getArg()->isVar())
					{
						const VarTerm &var = app.getArg()->asVar();

						auto itEnv = env.first.find(var.getVar());
						if (itEnv != env.first.end())
						{
							auto closurePtr = itEnv->second;
							Closure_t *closure = reinterpret_cast<Closure_t *>(closurePtr.get());

							if (closure->second->isVal())
							{
								m_Memory[loc].push_back(*closure);
								hasPushedAsValue = true;
							}
						}
					}

					if (!hasPushedAsValue)
					{
						m_Memory[loc].push_back(std::make_pair(env, app.getArg()));
					}
				}
			};

			auto itEnv = env.second.find(app.getLoc());
			if (itEnv != env.second.end())
			{
				appActionWithLoc(itEnv->second);
			}
			else if (isReservedLoc(app.getLoc()))
			{
				appActionWithLoc(app.getLoc());
			}
			else
			{
				machineError("Application cannot push to (invalid) location '"
					+ app.getLoc() + "' !", *this);
			}
		}
		else if (term->isAbs())
		{
			const AbsTerm &abs = term->asAbs();

			auto absActionWithLoc = [&](Loc_t loc) {
				// New stream
				if (loc == k_NewLoc)
				{
					Loc_t newLoc = locGenerator();
					m_Memory[newLoc] = {};

					if (abs.getVar())
					{
						env.first[abs.getVar().value()] = std::make_shared<Closure_t>(std::make_pair(
							Env_t{}, freshTerm(ValTerm(newLoc))
						));
					}

					m_Control.push_back(std::make_pair(env, abs.getBody()));
				}
				// Input stream
				else if (loc == k_InputLoc)
				{
					std::string in;
					std::cin >> in;

					Parser parser;
					if (auto termOpt = parser.parseTerm(in))
					{
						if (abs.getVar())
						{
							env.first[abs.getVar().value()] = std::make_shared<Closure_t>(std::make_pair(
								Env_t{}, freshTerm(std::move(termOpt.value()))
							));
						}

						m_Control.push_back(std::make_pair(env, abs.getBody()));
					}
					else
					{
						machineError("Cannot parse input '"
							+ in + "' as term !", *this);
					}
				}
				// Output stream
				else if (loc == k_OutputLoc)
				{
					machineError("Abstraction cannot bind from 'output' location !", *this);
				}
				// Null stream
				else if (loc == k_NullLoc)
				{
					machineError("Abstraction cannot bind from 'null' location !", *this);
				}
				// Generic stack
				else
				{
					if (auto closureOpt = tryPop(env, loc))
					{
						if (abs.getVar())
						{
							env.first[abs.getVar().value()] = std::make_shared<Closure_t>(closureOpt.value());
						}

						m_Control.push_back(std::make_pair(env, abs.getBody()));
					}
					else
					{
						machineError("Abstraction cannot pop from location '"
							+ loc + "' !", *this);
					}
				}
			};

			auto itEnv = env.second.find(abs.getLoc());
			if (itEnv != env.second.end())
			{
				absActionWithLoc(itEnv->second);
			}
			else if (isReservedLoc(abs.getLoc()))
			{
				absActionWithLoc(abs.getLoc());
			}
			else
			{
				machineError("Abstraction cannot pop from (invalid) location '"
					+ abs.getLoc() + "' !", *this);
			}
		}
		else if (term->isLocApp())
		{
			const LocAppTerm &locApp = term->asLocApp();

			m_Control.push_back(std::make_pair(env, locApp.getBody()));

			auto appActionWithLoc = [&](Loc_t loc) {
				// New stream
				if (loc == k_NewLoc)
				{
					machineError("Location application cannot push to 'new' location ! ", *this);
				}
				// Input stream
				else if (loc == k_InputLoc)
				{
					machineError("Location application cannot push to 'input' location ! ", *this);
				}
				// Null stream
				else if (loc == k_NullLoc)
				{}
				else
				{
					Loc_t locArg = locApp.getArg();

					auto itEnv = env.second.find(locApp.getArg());
					if (itEnv != env.second.end())
					{
						locArg = itEnv->second;
					}

					// Output stream
					if (loc == k_OutputLoc)
					{
						std::cout << stringifyClosure(std::make_pair(env, freshTerm(ValTerm(locArg)))) << std::endl;
					}
					// Generic stack
					else
					{
						m_Memory[loc].push_back(std::make_pair(env, freshTerm(ValTerm(locArg))));
					}
				}
			};

			auto itEnv = env.second.find(locApp.getLoc());
			if (itEnv != env.second.end())
			{
				appActionWithLoc(itEnv->second);
			}
			else if (isReservedLoc(locApp.getLoc()))
			{
				appActionWithLoc(locApp.getLoc());
			}
			else
			{
				machineError("Location application cannot push to (invalid) location '"
					+ locApp.getLoc() + "' !", *this);
			}
		}
		else if (term->isLocAbs())
		{
			const LocAbsTerm &locAbs = term->asLocAbs();
			
			auto absActionWithLoc = [&](Loc_t loc) {
				// New stream
				if (loc == k_NewLoc)
				{
					Loc_t newLoc = locGenerator();
					m_Memory[newLoc] = {};

					if (locAbs.getLocVar())
					{
						env.second[locAbs.getLocVar().value()] = newLoc;
					}

					m_Control.push_back(std::make_pair(env, locAbs.getBody()));
				}
				// Input stream
				else if (loc == k_InputLoc)
				{
					machineError("Location abstraction cannot pop from 'input' location !", *this);
				}
				// Output stream
				else if (loc == k_OutputLoc)
				{
					machineError("Location abstraction cannot pop from 'output' location !", *this);
				}
				// Null stream
				else if (loc == k_NullLoc)
				{
					machineError("Location abstraction cannot pop from 'null' location !", *this);
				}
				// Generic stack
				else
				{
					if (auto locOpt = tryPopLoc(env, loc))
					{
						if (locAbs.getLocVar())
						{
							env.second[locAbs.getLocVar().value()] = locOpt.value();
						}

						m_Control.push_back(std::make_pair(env, locAbs.getBody()));
					}
					else
					{
						machineError("Location abstraction cannot pop from location '"
							+ loc + "' !", *this);
					}
				}
			};

			auto itEnv = env.second.find(locAbs.getLoc());
			if (itEnv != env.second.end())
			{
				absActionWithLoc(itEnv->second);
			}
			else if (isReservedLoc(locAbs.getLoc()))
			{
				absActionWithLoc(locAbs.getLoc());
			}
			else
			{
				machineError("Location abstraction cannot pop from (invalid) location '"
					+ locAbs.getLoc() + "' !", *this);
			}
		}
		else if (term->isVal())
		{
			machineError("Value '" + stringifyClosure(closure)
				+ "' cannot be executed by machine !", *this);
		}
		else if (term->isBinOp())
		{
			const BinOpTerm &binOp = term->asBinOp();

			m_Control.push_back(std::make_pair(env, binOp.getBody()));

			if (auto prim1Opt = tryPopPrim(env, k_LambdaLoc))
			{
				if (auto prim2Opt = tryPopPrim(env, k_LambdaLoc))
				{
					auto prim1 = prim1Opt.value();
					auto prim2 = prim2Opt.value();

					if (binOp.isOp(BinOpTerm::Plus))
					{
						m_Memory[k_LambdaLoc].push_back(std::make_pair(env, freshTerm(ValTerm(prim2 + prim1))));
					}
					else if (binOp.isOp(BinOpTerm::Minus))
					{
						m_Memory[k_LambdaLoc].push_back(std::make_pair(env, freshTerm(ValTerm(prim2 - prim1))));
					}
				}
				else
				{
					machineError("Binary operation cannot use a non-primitive-value as second operand !", *this);
				}
			}
			else
			{
				machineError("Binary operation cannot use a non-primitive-value as first operand !", *this);
			}
		}
		else if (term->isPrimCases())
		{
			const CasesTerm<Prim_t> &cases = term->asPrimCases();

			m_Control.push_back(std::make_pair(env, cases.getBody()));

			if (auto primOpt = tryPopPrim(env, k_LambdaLoc))
			{
				auto itCase = cases.find(primOpt.value());
				if (itCase != cases.end())
				{
					m_Control.push_back(std::make_pair(env, itCase->second));
					
					m_CallStack.push_back({"Case '" + std::to_string(primOpt.value()) + "'", closure.second});
				}
				else
				{
					m_Control.push_back(std::make_pair(env, cases.getOtherwise()));
					
					m_CallStack.push_back({"Case 'otherwise'", closure.second});
				}
			}
			else
			{
				machineError("Primitive cases cannot match a non-primitive value !", *this);
			}
		}
		else if (term->isLocCases())
		{
			const CasesTerm<Loc_t> &cases = term->asLocCases();

			m_Control.push_back(std::make_pair(env, cases.getBody()));

			if (auto locOpt = tryPopLoc(env, k_LambdaLoc))
			{
				auto itCase = cases.find(locOpt.value());
				if (itCase != cases.end())
				{
					m_Control.push_back(std::make_pair(env, itCase->second));
					m_CallStack.push_back({"Case '" + locOpt.value() + "'", closure.second});
				}
				else
				{
					m_Control.push_back(std::make_pair(env, cases.getOtherwise()));	
					m_CallStack.push_back({"Case 'otherwise'", closure.second});
				}
			}
			else
			{
				machineError("Location cases cannot match a non-location value !", *this);
			}
		}
	}
}

std::optional<Closure_t> Machine::tryPop(Env_t env, Loc_t loc)
{
	if (!m_Memory[loc].empty())
	{
		Closure_t closure = m_Memory[loc].back();
		m_Memory[loc].pop_back();
		return closure;
	}
	else
	{
		machineError("Cannot pop from empty stack  '"
			+ loc + "' !", *this);
	}

	return std::nullopt;
}

std::optional<Prim_t> Machine::tryPopPrim(Env_t env, Loc_t loc)
{
	if (!m_Memory[loc].empty())
	{
		if (m_Memory[loc].back().second->isVal())
		{
			const ValTerm &val = m_Memory[loc].back().second->asVal();
			m_Memory[loc].pop_back();

			if (val.isPrim())
			{
				return val.asPrim();
			}
		}
	}
	else
	{
		machineError("Cannot pop from empty stack  '"
			+ loc + "' !", *this);
	}

	return std::nullopt;
}

std::optional<Loc_t> Machine::tryPopLoc(Env_t env, Loc_t loc)
{
	if (!m_Memory[loc].empty())
	{
		if (m_Memory[loc].back().second->isVal())
		{
			const ValTerm &val = m_Memory[loc].back().second->asVal();
			m_Memory[loc].pop_back();

			if (val.isLoc())
			{
				return val.asLoc();
			}
		}
	}
	else
	{
		machineError("Cannot pop from empty stack  '"
			+ loc + "' !", *this);
	}

	return std::nullopt;
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

	for (auto itMemory = m_Memory.begin(); itMemory != m_Memory.end(); ++itMemory)
	{
		if (auto idOpt = getIdFromReservedLoc(itMemory->first))
		{
			ss << "  -- (Reserved) Location " << idOpt.value() << '\n';
		}
		else
		{
			ss << "  -- Location " << itMemory->first << '\n';
		}

		for (auto itStack = itMemory->second.rbegin(); itStack != itMemory->second.rend(); ++itStack)
		{
			ss << "    " << stringifyClosure(*itStack) << '\n';
		}

		auto itMemoryCopy = itMemory;
		if (!(++itMemoryCopy == m_Memory.end()))
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

	for (auto itCallStack = m_CallStack.rbegin(); itCallStack != m_CallStack.rend(); ++itCallStack)
	{
		ss << std::string(m_CallStack.size() - (m_CallStack.rend() - itCallStack), ' ');
		ss << "> " << itCallStack->first << " => " << stringifyTerm(itCallStack->second) << "\n";
	}

	ss << "---------------";

	return ss.str();
}