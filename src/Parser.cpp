#include "Parser.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>

#include "Utils.hpp"

static void parseError(std::string message, const Lexer &lexer)
{
	const std::string &buf = lexer.getBuffer();
	const std::string &tokenbuf = lexer.getTokenBuffer();
	int tokenLen = tokenbuf.size();

	std::cerr << "[Parse Error]" << std::endl;

	std::stringstream ss(buf);
	std::string line;

	while (std::getline(ss, line, '\n'))
	{
		std::cerr << "|  " << line << std::endl;
	}

	std::string err;
	err += std::string(line.size() - tokenLen, ' ');
	err += std::string(tokenLen, '^');
	err += " ";
	err += message;

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

	m_Lexer->advance();

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

	m_Lexer->advance();

	while (m_Lexer->getToken() != Token::Eof)
	{
		if (m_Lexer->getToken() == Token::Id)
		{
			std::string id = m_Lexer->getTokenBuffer();

			m_Lexer->advance();

			if (m_Lexer->getToken() == Token::Equal)
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
							parseError("Expected ')' after definition of function '" + id + "'", *m_Lexer);
						}
					}
					else
					{
						parseError("Expected term for definition of function '" + id + "'", *m_Lexer);
					}
				}
				else
				{
					parseError("Expected '(' before definition of function '" + id + "'", *m_Lexer);
				}
			}
			else
			{
				parseError("Expected '=' after declaration of function '" + id + "'", *m_Lexer);
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
	if (m_Lexer->getToken() == Token::Asterisk)
	{
		m_Lexer->advance();

		return Term(NilTerm());
	}
	else if (auto varContOpt = parseVarCont())
	{
		return Term(std::move(varContOpt.value()));
	}
	else if (auto absOpt = parseAbstraction())
	{
		if (std::holds_alternative<AbsTerm>(absOpt.value()))
		{
			return Term(std::move(std::get<AbsTerm>(absOpt.value())));
		}
		else if (std::holds_alternative<LocAbsTerm>(absOpt.value()))
		{
			return Term(std::move(std::get<LocAbsTerm>(absOpt.value())));
		}
	}
	else if (auto absOpt = parseApplication())
	{
		if (std::holds_alternative<AppTerm>(absOpt.value()))
		{
			return Term(std::move(std::get<AppTerm>(absOpt.value())));
		}
		else if (std::holds_alternative<LocAppTerm>(absOpt.value()))
		{
			return Term(std::move(std::get<LocAppTerm>(absOpt.value())));
		}
	}
	else if (auto valOpt = parseVal())
	{
		return Term(std::move(valOpt.value()));
	}
	else if (auto casesOpt = parseCases())
	{
		return Term(std::move(casesOpt.value()));
	}

	return std::nullopt;
}

std::optional<VarContTerm> Parser::parseVarCont()
{
	Var_t var = m_Lexer->getTokenBuffer();

	if (m_Lexer->getToken() == Token::Id)
	{
		var = m_Lexer->getTokenBuffer();

		if (   m_Lexer->peekToken() == Token::Dot
			|| m_Lexer->peekToken() == Token::Comma
			|| m_Lexer->peekToken() == Token::Rsb
			|| m_Lexer->peekToken() == Token::Rb
			|| m_Lexer->peekToken() == Token::Eof)
		{
			m_Lexer->advance();
		}
	}

	if (m_Lexer->getToken() == Token::Dot)
	{
		m_Lexer->advance();

		if (auto bodyOpt = parseTerm())
		{
			return VarContTerm(var, std::move(bodyOpt.value()));
		}
		else
		{
			parseError("Expected term after variable '" + var + "'", *m_Lexer);
		}
	}
	else if (  m_Lexer->getToken() == Token::Comma
			|| m_Lexer->getToken() == Token::Rsb
			|| m_Lexer->getToken() == Token::Rb
			|| m_Lexer->getToken() == Token::Eof)
	{
		return VarContTerm(var);
	}

	return std::nullopt;
}

std::optional<std::variant<AbsTerm, LocAbsTerm>> Parser::parseAbstraction()
{
	Loc_t loc = Loc_t(k_DefaultLoc);

	if (m_Lexer->getToken() == Token::Id)
	{
		loc = m_Lexer->getTokenBuffer();

		if (m_Lexer->peekToken() == Token::Lab)
		{
			m_Lexer->advance();
		}
	}

	if (m_Lexer->getToken() == Token::Lab)
	{
		m_Lexer->advance();

		if (m_Lexer->getToken() == Token::Caret)
		{
			m_Lexer->advance();

			if (m_Lexer->getToken() == Token::Id)
			{
				LocVar_t locVar = m_Lexer->getTokenBuffer();

				m_Lexer->advance();

				if (m_Lexer->getToken() == Token::Rab)
				{
					m_Lexer->advance();

					LocAbsTerm locAbs(loc, locVar);

					if (m_Lexer->getToken() == Token::Dot)
					{
						m_Lexer->advance();

						if (auto bodyOpt = parseTerm())
						{
							*locAbs.body = Term(std::move(bodyOpt.value()));
						}
					}

					return locAbs;
				}
				else
				{
					parseError("Expected closing '>' of location abstraction", *m_Lexer);
				}
			}
			else
			{
				parseError("Expected binding location variable of location abstraction", *m_Lexer);
			}
		}
		else
		{
			std::optional<Var_t> varOpt;

			if (m_Lexer->getToken() == Token::Id)
			{
				varOpt = m_Lexer->getTokenBuffer();

				m_Lexer->advance();
			}
			else if (m_Lexer->getToken() == Token::Underscore)
			{
				varOpt = std::nullopt;

				m_Lexer->advance();
			}
			else
			{
				parseError("Expected binding variable of abstraction", *m_Lexer);
			}

			if (m_Lexer->getToken() == Token::Rab)
			{
				m_Lexer->advance();

				AbsTerm abs(loc);
				if (varOpt)
				{
					abs.var = varOpt.value();
				}

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
	}

	return std::nullopt;
}

std::optional<std::variant<AppTerm, LocAppTerm>> Parser::parseApplication()
{
	if (m_Lexer->getToken() == Token::Lsb)
	{
		m_Lexer->advance();

		if (m_Lexer->getToken() == Token::Hash)
		{
			m_Lexer->advance();

			if (m_Lexer->getToken() == Token::Id)
			{
				LocVar_t locVar = m_Lexer->getTokenBuffer();

				m_Lexer->advance();

				if (m_Lexer->getToken() == Token::Rsb)
				{
					m_Lexer->advance();

					Loc_t loc = Loc_t(k_DefaultLoc);

					if (m_Lexer->getToken() == Token::Id)
					{
						loc = m_Lexer->getTokenBuffer();

						m_Lexer->advance();
					}
					
					LocAppTerm locApp(loc, locVar);

					if (m_Lexer->getToken() == Token::Dot)
					{
						m_Lexer->advance();

						if (auto bodyOpt = parseTerm())
						{
							*locApp.body = Term(std::move(bodyOpt.value()));
						}
					}

					return locApp;
				}
				else
				{
					parseError("Expected location for location application", *m_Lexer);
				}
			}
		}
		else if (auto argOpt = parseTerm())
		{
			if (m_Lexer->getToken() == Token::Rsb)
			{
				m_Lexer->advance();

				Loc_t loc = Loc_t(k_DefaultLoc);

				if (m_Lexer->getToken() == Token::Id)
				{
					loc = m_Lexer->getTokenBuffer();

					m_Lexer->advance();
				}
				
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

std::optional<ValTerm> Parser::parseVal()
{
	if (m_Lexer->getToken() == Token::Primitive)
	{
		std::string primStr = m_Lexer->getTokenBuffer();

		m_Lexer->advance();

		int primInt = std::stoi(primStr);
		Prim_t prim = static_cast<Prim_t>(primInt);
		return ValTerm(prim);
	}

	return std::nullopt;
}

std::optional<std::pair<std::string, Term>> Parser::parseCase()
{
	if (m_Lexer->getToken() == Token::Id)
	{
		std::string id = m_Lexer->getTokenBuffer();

		m_Lexer->advance();

		if (m_Lexer->getToken() == Token::Arrow)
			{
				m_Lexer->advance();

				if (auto termOpt = parseTerm())
				{
					return std::make_pair(id, std::move(termOpt.value()));
				}
				else
				{
					parseError("Expected term for case mapping", *m_Lexer);
				}
			}
			else
			{
				parseError("Expected '->' for case", *m_Lexer);
			}
	}
	else
	{
		parseError("Expected pattern for case", *m_Lexer);
	}

	return std::nullopt;
}

std::optional<CasesTerm> Parser::parseCases()
{
	if (m_Lexer->getToken() == Token::Lb)
	{
		m_Lexer->advance();

		CasesTerm cases;
		bool isCaseRemaining = true;

		while (isCaseRemaining)
		{
			if (auto caseOpt = parseCase())
			{
				auto &[val, term] = caseOpt.value();
				cases.addCase(val, std::move(term));

				if (m_Lexer->getToken() == Token::Comma)
				{
					m_Lexer->advance();

					isCaseRemaining = true;
				}
				else
				{
					isCaseRemaining = false;
				}
			}
			else
			{
				parseError("Expected case mapping for cases", *m_Lexer);
			}
		}

		if (m_Lexer->getToken() == Token::Rb)
		{
			m_Lexer->advance();

			if (m_Lexer->getToken() == Token::Dot)
			{
				m_Lexer->advance();

				if (auto bodyOpt = parseTerm())
				{
					*cases.body = Term(std::move(bodyOpt.value()));
				}
			}

			return cases;
		}
		else
		{
			parseError("Expected closing ')' for cases", *m_Lexer);
		}
	}

	return std::nullopt;
}