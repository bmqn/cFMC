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

static void parseError(std::string message, const Lexer *lexer = nullptr)
{
	std::cerr << "[Parse Error] " << message << std::endl;

	if (lexer)
	{
		std::string buf = lexer->getFullBuffer();
		int tokenLen = lexer->getTokenBuffer().size();

		std::string underlineErr;
		underlineErr += std::string(buf.size() - tokenLen, ' ');
		underlineErr += std::string("~");

		std::cerr << "    " << buf << std::endl;
		std::cerr << "    " << underlineErr << std::endl;
	}

	std::exit(1);
}


Parser::Parser() : m_Lexer(nullptr) {}

Program Parser::parseProgram(const std::string &programSrc)
{
	std::istringstream iss(programSrc);
	m_Lexer = std::make_unique<Lexer>(iss);

	FuncDefs_t funcs = parseFuncDefs();
	return Program(std::move(funcs));
}

Term Parser::parseTerm(const std::string &programSrc)
{
	std::istringstream iss(programSrc);
	m_Lexer = std::make_unique<Lexer>(iss);

	if (m_Lexer->getToken() != Token::Eof)
	{
		if (auto termOpt = parseTerm(false))
		{
			return std::move(termOpt.value());
		}
	}

	return Term();
}

FuncDefs_t Parser::parseFuncDefs()
{
	FuncDefs_t funcs;

	while (m_Lexer->getToken() != Token::Eof)
	{
		if (m_Lexer->getToken() == Token::Id)
		{
			std::string id = m_Lexer->getTokenBuffer();

			m_Lexer->advance();

			if (m_Lexer->getToken() == Token::Eql)
			{
				m_Lexer->advance();

				if (m_Lexer->getToken() == Token::Lb)
				{
					m_Lexer->advance();

					if (auto termOpt = parseTerm())
					{
						if (m_Lexer->getToken() == Token::Rb)
						{
							m_Lexer->advance();

							funcs.emplace(id, std::move(termOpt.value()));
						}
						else
						{
							parseError(
								"Expected ')' after definition of function declaration '" + id + "'.",
								m_Lexer.get()
							);
						}
					}
					else
					{
						parseError(
							"Expected definition of function declaration '" + id + "'.",
							m_Lexer.get()
						);
					}
				}
				else
				{
					parseError(
								"Expected '(' before definition of function declaration '" + id + "'.",
								m_Lexer.get()
							);
				}
			}
			else
			{
				parseError(
					"Expected '=' after function declaration '" + id + "'.",
					m_Lexer.get()
				);
			}
		}
		else
		{
			parseError(
				"Expected function declaration.",
				m_Lexer.get()
			);
		}
	}

	return funcs;
}

std::optional<Term> Parser::parseTerm(bool handleErrors)
{
	if (m_Lexer->getToken() == Token::Eof)
	{
		return Term(NilTerm());
	}
	else if (m_Lexer->getToken() == Token::Astx)
	{
		m_Lexer->advance();

		return Term(NilTerm());
	} 
	else if (auto absOpt = parseAbstraction())
	{
		return Term(std::move(absOpt.value()));
	}
	else if (auto appOpt = parseApplication())
	{
		return Term(std::move(appOpt.value()));
	}
	else if (m_Lexer->getToken() == Token::Id)
	{
		std::string id = m_Lexer->getTokenBuffer();

		m_Lexer->advance();

		if (auto absOpt = parseAbstraction(Loc(id)))
		{
			return Term(std::move(absOpt.value()));
		}
		else if (auto appOpt = parseApplication(Loc(id)))
		{
			return Term(std::move(appOpt.value()));
		}
		else
		{
			VarContTerm varCont(id);

			if (m_Lexer->getToken() == Token::Dot)
			{
				m_Lexer->advance();

				if (auto bodyOpt = parseTerm())
				{
					*varCont.body = Term(std::move(bodyOpt.value()));
				}
				else
				{
					parseError("Expected term or nothing.", m_Lexer.get());
				}
			}

			return Term(std::move(varCont));
		}
	}
	else if (handleErrors)
	{
		parseError("Expected term.", m_Lexer.get());
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
			std::string id = m_Lexer->getTokenBuffer();

			m_Lexer->advance();

			if (m_Lexer->getToken() == Token::Rab)
			{
				m_Lexer->advance();

				AbsTerm abs(loc, id);

				if (m_Lexer->getToken() == Token::Dot)
				{
					m_Lexer->advance();

					if (auto bodyOpt = parseTerm())
					{
						*abs.body = Term(std::move(bodyOpt.value()));
					}
				}

				return abs;
			}
			else
			{
				parseError("Expected closing '>' after abstraction.", m_Lexer.get());
			}
		}
		else
		{
			parseError("Expected binding variable for abstraction.", m_Lexer.get());
		}
	}

	return std::nullopt;
}

std::optional<AppTerm> Parser::parseApplication(Loc loc)
{
	if (m_Lexer->getToken() == Token::Lsb)
	{
		m_Lexer->advance();

		if (auto argOpt = parseTerm(false))
		{
			if (m_Lexer->getToken() == Token::Rsb)
			{
				m_Lexer->advance();

				AppTerm app(loc);
				*app.arg = Term(std::move(argOpt.value()));

				if (m_Lexer->getToken() == Token::Dot)
				{
					m_Lexer->advance();

					if (auto bodyOpt = parseTerm())
					{
						*app.body = Term(std::move(bodyOpt.value()));
					}
				}

				return app;
			}
			else
			{
				parseError("Expected closing ']' after application.", m_Lexer.get());
			}
		}
		else
		{
			parseError("Expected inner term for abstraction.", m_Lexer.get());
		}
	}

	return std::nullopt;
}