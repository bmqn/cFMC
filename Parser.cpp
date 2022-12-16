#include "Parser.hpp"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdlib>

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

static void parseError(std::string message, const Lexer &lexer)
{
	std::string complBuf = lexer.getCompleteBuffer();
	int tokenLen = lexer.getTokenText().size();

	std::string underlineErr;
	underlineErr += std::string(complBuf.size() - tokenLen, ' ');
	underlineErr += std::string(tokenLen, '~');

	std::cerr << "[Parse Error] " << message << std::endl;
	std::cerr << "    " << complBuf << std::endl;
	std::cerr << "    " << underlineErr << std::endl;

	std::exit(1);
}

VarContTerm::VarContTerm() : var(), body(new Term(Term::Nil, NilTerm())) {}
VarContTerm::VarContTerm(Var var) : var(var), body(new Term(Term::Nil, NilTerm())) {}

AbsTerm::AbsTerm() : loc(), var(), body(new Term(Term::Nil, NilTerm())) {}

AppTerm::AppTerm() : loc(), arg(new Term(Term::Nil, NilTerm())), body(new Term(Term::Nil, NilTerm())) {}

Parser::Parser()
	: m_Lexer(nullptr)
{
}

Program Parser::parse(const std::string &programSrc)
{
	std::istringstream iss(programSrc);
	m_Lexer = std::make_unique<Lexer>(iss);

	Program program;
	program.setEntry(parseMain().value_or(Term(Term::Nil, NilTerm())));

	return program;
}

std::optional<Term> Parser::parseMain()
{
	if (m_Lexer->getToken() == Token::Id)
	{
		std::string id = m_Lexer->getTokenText();

		m_Lexer->advance();

		if (id == "main")
		{
			if (m_Lexer->getToken() == Token::StaticFuncDec)
			{
				m_Lexer->advance();

				if (auto termOpt = parseTerm())
				{
					Term term = termOpt.value();
					return term;
				}
				else
				{
					std::cerr << "[Parse Error] Expected function definition but got '" << m_Lexer->getCompleteBuffer() << "' instead." << std::endl;
				}
			}
		}
	}

	return std::nullopt;
}

std::optional<Term> Parser::parseTerm()
{
	if (m_Lexer->getToken() == Token::Nil)
	{
		m_Lexer->advance();

		return Term(Term::Nil, NilTerm());
	}
	else if (m_Lexer->getToken() == Token::Eof)
	{
		return Term(Term::Nil, NilTerm());
	}
	else if (auto absOpt = parseAbstraction())
	{
		AbsTerm abs = absOpt.value();
		return Term(Term::Abs, abs);
	}
	else if (auto appOpt = parseApplication())
	{
		AppTerm app = appOpt.value();
		return Term(Term::App, app);
	}
	else if (m_Lexer->getToken() == Token::Id)
	{
		std::string id = m_Lexer->getTokenText();

		m_Lexer->advance();

		if (auto absOpt = parseAbstraction(Loc(id)))
		{
			AbsTerm abs = absOpt.value();
			return Term(Term::Abs, abs);
		}
		else if (auto appOpt = parseApplication(Loc(id)))
		{
			AppTerm app = appOpt.value();
			return Term(Term::App, app);
		}
		else
		{
			VarContTerm varCont;
			varCont.var = Var(id);
			varCont.body = new Term(Term::Nil, NilTerm());

			if (m_Lexer->getToken() == Token::Dot)
			{
				m_Lexer->advance();

				if (auto bodyOpt = parseTerm())
				{
					Term body = bodyOpt.value();
					varCont.body = new Term(body);
				}
				else
				{
					parseError("Expected term or nothing.", *m_Lexer);
				}
			}

			return Term(Term::VarCont, varCont);
		}
	}
	else if (m_Lexer->getToken() == Token::True)
	{
		m_Lexer->advance();
		
		// true := <x>.<y>.x

		std::string varx = varGenerator(); // x
		std::string vary = varGenerator(); // y

		VarContTerm varxCont; // x
		varxCont.var = varx;
		varxCont.body = new Term(Term::Nil, NilTerm());

		if (m_Lexer->getToken() == Token::Dot)
		{
			m_Lexer->advance();

			if (auto bodyOpt = parseTerm())
			{
				Term body = bodyOpt.value();
				varxCont.body = new Term(body);
			}
		}
		else
		{
			if (m_Lexer->getToken() != Token::Eof &&
			    m_Lexer->getToken() != Token::Rab &&
				m_Lexer->getToken() != Token::Rsb)
			{
				parseError("Expected term or nothing after 'true''.", *m_Lexer);
			}
		}

		AbsTerm absy; // <y>
		absy.loc = k_DefaultLoc;
		absy.var = vary;
		absy.body = new Term(Term::VarCont, varxCont);

		AbsTerm absx; // <x>
		absx.loc = k_DefaultLoc;
		absx.var = varx;
		absx.body = new Term(Term::Abs, absy);

		return Term(Term::Abs, absx);
	}
	else if (m_Lexer->getToken() == Token::False)
	{
		m_Lexer->advance();

		// false := <x>.<y>.y

		std::string varx = varGenerator(); // x
		std::string vary = varGenerator(); // y

		VarContTerm varyCont; // y
		varyCont.var = vary;
		varyCont.body = new Term(Term::Nil, NilTerm());

		if (m_Lexer->getToken() == Token::Dot)
		{
			m_Lexer->advance();

			if (auto bodyOpt = parseTerm())
			{
				Term body = bodyOpt.value();
				varyCont.body = new Term(body);
			}
		}
		else
		{
			if (m_Lexer->getToken() != Token::Eof &&
			    m_Lexer->getToken() != Token::Rab &&
				m_Lexer->getToken() != Token::Rsb)
			{
				parseError("Expected term or nothing after 'false''.", *m_Lexer);
			}
		}

		AbsTerm absy; // <y>
		absy.loc = k_DefaultLoc;
		absy.var = vary;
		absy.body = new Term(Term::VarCont, varyCont);

		AbsTerm absx; // <x>
		absx.loc = k_DefaultLoc;
		absx.var = varx;
		absx.body = new Term(Term::Abs, absy);

		return Term(Term::Abs, absx);
	}
	else if (m_Lexer->getToken() == Token::IfFunc)
	{
		m_Lexer->advance();

		// if := <p>.<a>.<b>.[b].[a].p

		std::string varp = varGenerator(); // p
		std::string vara = varGenerator(); // a
		std::string varb = varGenerator(); // b

		VarContTerm varpCont; // p
		varpCont.var = varp;
		varpCont.body = new Term(Term::Nil, NilTerm());

		if (m_Lexer->getToken() == Token::Dot)
		{
			m_Lexer->advance();

			if (auto bodyOpt = parseTerm())
			{
				Term body = bodyOpt.value();
				varpCont.body = new Term(body);
			}
		}
		else
		{
			if (m_Lexer->getToken() != Token::Eof &&
			    m_Lexer->getToken() != Token::Rab &&
				m_Lexer->getToken() != Token::Rsb)
			{
				parseError("Expected term or nothing after 'if''.", *m_Lexer);
			}
		}

		VarContTerm varaCont; // a
		varaCont.var = vara;
		varaCont.body = new Term(Term::Nil, NilTerm());

		VarContTerm varbCont; // b
		varbCont.var = varb;
		varbCont.body = new Term(Term::Nil, NilTerm());

		AppTerm appa;
		appa.loc = k_DefaultLoc;
		appa.arg = new Term(Term::VarCont, varaCont);
		appa.body =  new Term(Term::VarCont, varpCont);

		AppTerm appb;
		appb.loc = k_DefaultLoc;
		appb.arg = new Term(Term::VarCont, varbCont);
		appb.body = new Term(Term::App, appa);

		AbsTerm absb; // <b>
		absb.loc = k_DefaultLoc;
		absb.var = varb;
		absb.body = new Term(Term::App, appb);

		AbsTerm absa; // <a>
		absa.loc = k_DefaultLoc;
		absa.var = vara;
		absa.body = new Term(Term::Abs, absb);

		AbsTerm absp; // <p>
		absp.loc = k_DefaultLoc;
		absp.var = varp;
		absp.body = new Term(Term::Abs, absa);

		return Term(Term::Abs, absp);
	}
	else if (m_Lexer->getToken() == Token::PrintFunc)
	{
		m_Lexer->advance();

		// print := <x>.out[x]

		std::string varx = varGenerator(); // x

		VarContTerm varxCont; // x
		varxCont.var = varx;
		varxCont.body = new Term(Term::Nil, NilTerm());

		AppTerm app; // out[x]
		app.loc = k_OutputLoc;
		app.arg = new Term(Term::VarCont, varxCont);
		app.body = new Term(Term::Nil, NilTerm());

		if (m_Lexer->getToken() == Token::Dot)
		{
			m_Lexer->advance();

			if (auto bodyOpt = parseTerm())
			{
				Term body = bodyOpt.value();
				app.body = new Term(body);
			}
		}
		else
		{
			if (m_Lexer->getToken() != Token::Eof &&
			    m_Lexer->getToken() != Token::Rab &&
				m_Lexer->getToken() != Token::Rsb)
			{
				parseError("Expected term or nothing after 'print'.", *m_Lexer);
			}
		}

		AbsTerm abs; // <x>
		abs.loc = k_DefaultLoc;
		abs.var = varx;
		abs.body = new Term(Term::App, app);

		return Term(Term::Abs, abs);
	}
	else
	{
		parseError("Expected term.", *m_Lexer);
	}

	return std::nullopt;
}

std::optional<AbsTerm> Parser::parseAbstraction(Loc loc)
{
	if (m_Lexer->getToken() == Token::Lab)
	{
		m_Lexer->advance();

		if (m_Lexer->getToken() == Token::Id)
		{
			std::string id = m_Lexer->getTokenText();

			m_Lexer->advance();

			if (m_Lexer->getToken() == Token::Rab)
			{
				m_Lexer->advance();

				AbsTerm abs;
				abs.loc = loc;
				abs.var = id;
				abs.body = new Term(Term::Nil, NilTerm());

				if (m_Lexer->getToken() == Token::Dot)
				{
					m_Lexer->advance();

					if (auto bodyOpt = parseTerm())
					{
						Term body = bodyOpt.value();
						abs.body = new Term(body);
					}
				}

				return abs;
			}
			else
			{
				parseError("Expected closing '>' for abstraction.", *m_Lexer);
			}
		}
		else
		{
			parseError("Expected variable for abstraction.", *m_Lexer);
		}
	}

	return std::nullopt;
}

std::optional<AppTerm> Parser::parseApplication(Loc loc)
{
	if (m_Lexer->getToken() == Token::Lsb)
	{
		m_Lexer->advance();

		if (auto argOpt = parseTerm())
		{
			Term arg = argOpt.value();

			if (m_Lexer->getToken() == Token::Rsb)
			{
				m_Lexer->advance();

				AppTerm app;
				app.loc = loc;
				app.arg = new Term(arg);
				app.body = new Term(Term::Nil, NilTerm());

				if (m_Lexer->getToken() == Token::Dot)
				{
					m_Lexer->advance();

					if (auto bodyOpt = parseTerm())
					{
						Term body = bodyOpt.value();
						app.body = new Term(body);
					}
				}

				return app;
			}
			else
			{
				parseError("Expected closing ']' for application.", *m_Lexer);
			}
		}
	}

	return std::nullopt;
}