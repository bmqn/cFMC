#include "Parser.hpp"

#include <iostream>
#include <iomanip>
#include <cstdlib>

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

static void parseError(std::string message, const Lexer &lexer)
{
	const std::string &src = lexer.getSource();
	const std::string &buf = lexer.getBuffer();
	const std::string &tokenbuf = lexer.getTokenBuffer();

	int tokenLen = tokenbuf.size();

	std::string err;
	err += std::string(buf.size() - tokenLen, ' ');
	err += std::string(tokenLen, '^');
	err += " ";
	err += message;

	std::cerr << "[Parse Error]" << std::endl;
	std::cerr << "|  " << src << std::endl;
	std::cerr << "|  " << err << std::endl;

	std::exit(1);
}

Parser::Parser() : m_Lexer(nullptr) {}

Program Parser::parseProgram(const std::string &programSrc)
{
	m_Lexer = std::make_unique<Lexer>(programSrc);

	FuncDefs_t funcs = parseFuncDefs();
	return Program(std::move(funcs));
}

Term Parser::parseTerm(const std::string &termSrc)
{
	m_Lexer = std::make_unique<Lexer>(termSrc);

	if (m_Lexer->getToken() != Token::Eof)
	{
		if (auto termOpt = parseTerm())
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
							parseError("Expected ')' after function definition of '" + id + "'", *m_Lexer);
						}
					}
					else
					{
						parseError("Expected term as function definition of '" + id + "'", *m_Lexer);
					}
				}
				else
				{
					parseError("Expected '(' before function definition of '" + id + "'", *m_Lexer);
				}
			}
			else
			{
				parseError("Expected '=' after function declaration of '" + id + "'", *m_Lexer);
			}
		}
		else
		{
			parseError("Expected function declaration", *m_Lexer);
		}
	}

	return funcs;
}

std::optional<Term> Parser::parseTerm()
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

		if (   id == k_DefaultLocId || id == k_NewLocId
			|| id == k_InputLocId || id == k_OutputLocId)
		{
			if (auto locOpt = getLocFromId(id))
			{
				if (auto absOpt = parseAbstraction(locOpt.value()))
				{
					return Term(std::move(absOpt.value()));
				}
				else if (auto appOpt = parseApplication(locOpt.value()))
				{
					return Term(std::move(appOpt.value()));
				}
				else
				{
					return Term(ValTerm(locOpt.value()));
				}
			}
			else
			{
				parseError("Expected valid location", *m_Lexer);
			}
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
					parseError("Expected term (or nothing)", *m_Lexer);
				}
			}

			return Term(std::move(varCont));
		}
	}

	return std::nullopt;
}

std::optional<AbsTerm> Parser::parseAbstraction(Loc_t loc)
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
				parseError("Expected closing '>' of abstraction", *m_Lexer);
			}
		}
		else
		{
			parseError("Expected binding variable of abstraction", *m_Lexer);
		}
	}

	return std::nullopt;
}

std::optional<AppTerm> Parser::parseApplication(Loc_t loc)
{
	if (m_Lexer->getToken() == Token::Lsb)
	{
		m_Lexer->advance();

		if (auto argOpt = parseTerm())
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
				parseError("Expected closing ']' of application", *m_Lexer);
			}
		}
		else
		{
			parseError("Expected inner term of application", *m_Lexer);
		}
	}

	return std::nullopt;
}