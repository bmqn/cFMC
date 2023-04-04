#include "Parser.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>

#include "Utils.hpp"

static void parseError(std::string message, const Lexer &lexer)
{
	if (auto tokenBufferOpt = lexer.getPeekBuffer())
	{
		std::string tokenBuffer = tokenBufferOpt.value();
		int tokenBufferLen = tokenBuffer.size();

		std::cerr << "[Parse Error]" << std::endl;

		const std::string buffer = lexer.getBuffer();
		size_t tokenBufferPos = 0;

		std::stringstream ss(buffer);
		std::string line;
		while (std::getline(ss, line, '\n'))
		{
			std::cerr << "|  " << line << std::endl;
			tokenBufferPos = line.find_last_of(tokenBuffer);
		}

		std::string err1;
		err1 += std::string(tokenBufferPos, ' ');
		err1 += std::string(tokenBufferLen, '^');
		err1 += " ";
		err1 += message;

		std::string err2;
		err2 += std::string(tokenBufferPos, ' ');
		err2 += "| Got '";
		err2 += tokenBuffer;
		err2 += "'";

		std::cerr << "|  " << err1 << std::endl;
		std::cerr << "|  " << err2 << std::endl;
	}

	std::exit(1);
}

Parser::Parser() : m_Lexer(nullptr) {}

Program Parser::parseProgram(const std::string &programSrc)
{
	m_Lexer = std::make_unique<Lexer>(programSrc);

	Program::FuncDefs_t funcs = parseFuncDefs();
	return Program(std::move(funcs));
}

std::optional<Term> Parser::parseTerm(const std::string &termSrc)
{
	m_Lexer = std::make_unique<Lexer>(termSrc);

	if (auto termOpt = parseTerm())
	{
		return std::move(termOpt.value());
	}

	return std::nullopt;
}

Program::FuncDefs_t Parser::parseFuncDefs()
{
	Program::FuncDefs_t funcs;
	
	while (!m_Lexer->isPeekToken(Token::Eof))
	{
		if (m_Lexer->isPeekToken(Token::Id))
		{
			if (auto funcOpt = m_Lexer->getPeekBuffer())
			{
				m_Lexer->next();

				if (m_Lexer->isPeekToken(Token::Equal))
				{
					m_Lexer->next();

					if (m_Lexer->isPeekToken(Token::Lb))
					{
						m_Lexer->next();

						if (auto termOpt = parseTerm())
						{
							if (m_Lexer->isPeekToken(Token::Rb))
							{
								m_Lexer->next();
								funcs.emplace(funcOpt.value(), newTerm(std::move(termOpt.value())));
							}
							else
							{
								parseError("Expected ')' after definition of function '" + funcOpt.value() + "'", *m_Lexer);
							}
						}
						else
						{
							parseError("Expected term for definition of function '" + funcOpt.value() + "'", *m_Lexer);
						}
					}
					else
					{
						parseError("Expected '(' before definition of function '" + funcOpt.value() + "'", *m_Lexer);
					}
				}
				else
				{
					parseError("Expected '=' after declaration of function '" + funcOpt.value() + "'", *m_Lexer);
				}
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
	if (m_Lexer->isPeekToken(Token::Asterisk))
	{
		m_Lexer->next();
		return Term(NilTerm());
	}
	else if (auto varOpt = parseVar())
	{
		return Term(std::move(varOpt.value()));
	}
	else if(auto locAbsOpt = parseLocAbs())
	{
		return Term(std::move(locAbsOpt.value()));
	}
	else if (auto locAppOpt = parseLocApp())
	{
		return Term(std::move(locAppOpt.value()));
	}
	else if (auto absOpt = parseAbs())
	{
		return Term(std::move(absOpt.value()));
	}
	else if (auto appOpt = parseApp())
	{
		return Term(std::move(appOpt.value()));
	}
	else if (auto valOpt = parseVal())
	{
		return Term(std::move(valOpt.value()));
	}
	// else if (auto primCasesOpt = parsePrimCases())
	// {
	// 	return Term(std::move(primCasesOpt.value()));
	// }
	else if (auto locCasesOpt = parseLocCases())
	{
		return Term(std::move(locCasesOpt.value()));
	}

	return std::nullopt;
}

std::optional<VarTerm> Parser::parseVar()
{
	if (m_Lexer->isPeekToken(Token::Id))
	{
		if (auto varOpt = m_Lexer->getPeekBuffer())
		{
			if (m_Lexer->isPeekToken(Token::Dot, 1))
			{
				m_Lexer->next();
				m_Lexer->next();
				
				if (auto bodyOpt = parseTerm())
				{
					return VarTerm(varOpt.value(), std::move(bodyOpt.value()));
				}
				else
				{
					parseError("Expected term after variable '" + varOpt.value() + "'", *m_Lexer);
				}
			}
			else if (m_Lexer->isPeekToken(Token::Comma, 1) ||
					 m_Lexer->isPeekToken(Token::Rsb, 1) ||
					 m_Lexer->isPeekToken(Token::Rb, 1) ||
					 m_Lexer->isPeekToken(Token::Eof, 1))
			{
				m_Lexer->next();
				return VarTerm(varOpt.value());
			}
		}
	}

	return std::nullopt;
}

std::optional<AbsTerm> Parser::parseAbs()
{
	std::optional<std::string> locOpt;

	if (m_Lexer->isPeekToken(Token::Id) &&
		m_Lexer->isPeekToken(Token::Lab, 1) &&
		!m_Lexer->isPeekToken(Token::Caret, 2))
	{
		locOpt = m_Lexer->getPeekBuffer();
		m_Lexer->next();
	}

	if (m_Lexer->isPeekToken(Token::Lab) &&
		!m_Lexer->isPeekToken(Token::Caret, 1))
	{
		m_Lexer->next();

		std::optional<std::string> varOpt;
		
		if (m_Lexer->isPeekToken(Token::Id))
		{
			varOpt = m_Lexer->getPeekBuffer();
			m_Lexer->next();
		}
		else if (m_Lexer->isPeekToken(Token::Underscore))
		{
			m_Lexer->next();
		}
		else
		{
			parseError("Expected binding variable of abstraction", *m_Lexer);
		}

		if (m_Lexer->isPeekToken(Token::Rab))
		{
			m_Lexer->next();

			if (m_Lexer->isPeekToken(Token::Dot))
			{
				m_Lexer->next();
				
				if (auto bodyOpt = parseTerm())
				{
					return AbsTerm(
						locOpt.value_or(k_DefaultLoc), varOpt,
						std::move(bodyOpt.value())
					);
				}
				else
				{
					parseError("Expected body of abstraction", *m_Lexer);
				}
			}

			return AbsTerm(
				locOpt.value_or(k_DefaultLoc), varOpt
			);
		}
		else
		{
			parseError("Expected closing '>' of abstraction", *m_Lexer);
		}
	}

	return std::nullopt;
}

std::optional<AppTerm> Parser::parseApp()
{
	if (m_Lexer->isPeekToken(Token::Lsb) &&
		!m_Lexer->isPeekToken(Token::Hash, 1))
	{
		m_Lexer->next();
		
		if (auto argOpt = parseTerm())
		{
			if (m_Lexer->isPeekToken(Token::Rsb))
			{
				m_Lexer->next();

				std::optional<std::string> locOpt;

				if (m_Lexer->isPeekToken(Token::Id))
				{
					locOpt = m_Lexer->getPeekBuffer();
					m_Lexer->next();
				}

				if (m_Lexer->isPeekToken(Token::Dot))
				{
					m_Lexer->next();
					
					if (auto bodyOpt = parseTerm())
					{
						return AppTerm(
							locOpt.value_or(k_DefaultLoc),
							std::move(argOpt.value()),
							std::move(bodyOpt.value())
						);
					}
					else
					{
						parseError("Expected body of application", *m_Lexer);
					}
				}

				return AppTerm(
					locOpt.value_or(k_DefaultLoc),
					std::move(argOpt.value())
				);
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

std::optional<LocAbsTerm> Parser::parseLocAbs()
{
	std::optional<std::string> locOpt;

	if (m_Lexer->isPeekToken(Token::Id) &&
		m_Lexer->isPeekToken(Token::Lab, 1) &&
		m_Lexer->isPeekToken(Token::Caret, 2))
	{
		locOpt = m_Lexer->getPeekBuffer();
		m_Lexer->next();
	}

	if (m_Lexer->isPeekToken(Token::Lab) &&
		m_Lexer->isPeekToken(Token::Caret, 1))
	{
		m_Lexer->next();
		m_Lexer->next();

		std::optional<std::string> varOpt;
		
		if (m_Lexer->isPeekToken(Token::Id))
		{
			varOpt = m_Lexer->getPeekBuffer();
			m_Lexer->next();
		}
		else if (m_Lexer->isPeekToken(Token::Underscore))
		{
			m_Lexer->next();
		}
		else
		{
			parseError("Expected binding variable of abstraction", *m_Lexer);
		}

		if (m_Lexer->isPeekToken(Token::Rab))
		{
			m_Lexer->next();
			
			if (m_Lexer->isPeekToken(Token::Dot))
			{
				m_Lexer->next();

				if (auto bodyOpt = parseTerm())
				{
					return LocAbsTerm(
						locOpt.value_or(k_DefaultLoc), varOpt,
						std::move(bodyOpt.value())
					);
				}
				else
				{
					parseError("Expected body of abstraction", *m_Lexer);
				}
			}

			return LocAbsTerm(
				locOpt.value_or(k_DefaultLoc), varOpt
			);
		}
		else
		{
			parseError("Expected closing '>' of abstraction", *m_Lexer);
		}
	}

	return std::nullopt;
}

std::optional<LocAppTerm> Parser::parseLocApp()
{
	if (m_Lexer->isPeekToken(Token::Lsb) &&
		m_Lexer->isPeekToken(Token::Hash, 1))
	{
		m_Lexer->next();
		m_Lexer->next();
		
		if (m_Lexer->isPeekToken(Token::Id))
		{
			if (auto argOpt = m_Lexer->getPeekBuffer())
			{
				m_Lexer->next();

				if (m_Lexer->isPeekToken(Token::Rsb))
				{
					m_Lexer->next();

					std::optional<std::string> locOpt;

					if (m_Lexer->isPeekToken(Token::Id))
					{
						locOpt = m_Lexer->getPeekBuffer();
						m_Lexer->next();
					}

					if (m_Lexer->isPeekToken(Token::Dot))
					{
						m_Lexer->next();
						
						if (auto bodyOpt = parseTerm())
						{
							return LocAppTerm(
								locOpt.value_or(k_DefaultLoc),
								argOpt.value(),
								std::move(bodyOpt.value())
							);
						}
						else
						{
							parseError("Expected body of application", *m_Lexer);
						}
					}

					return LocAppTerm(
						locOpt.value_or(k_DefaultLoc),
						argOpt.value()
					);
				}
				else
				{
					parseError("Expected closing ']' of application", *m_Lexer);
				}
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
	if (m_Lexer->isPeekToken(Token::Primitive))
	{
		if (auto primOpt = m_Lexer->getPeekBuffer())
		{
			m_Lexer->next();

			int prim = std::stoi(primOpt.value());
			return ValTerm(
				prim
			);
		}
	}

	return std::nullopt;
}

std::optional<std::pair<Loc_t, Term>> Parser::parseLocCase()
{
	if (m_Lexer->isPeekToken(Token::Id))
	{
		if (auto locOpt = m_Lexer->getPeekBuffer())
		{
			m_Lexer->next();

			if (m_Lexer->isPeekToken(Token::Arrow))
			{
				m_Lexer->next();

				if (auto termOpt = parseTerm())
				{
					return std::make_pair(locOpt.value(), std::move(termOpt.value()));
				}
				else
				{
					parseError("Expected mapping term for case", *m_Lexer);
				}
			}
			else
			{
				parseError("Expected mapping '->' for case", *m_Lexer);
			}
		}
	}
	else
	{
		parseError("Expected pattern to match for case", *m_Lexer);
	}

	return std::nullopt;
}

std::optional<CasesTerm<Loc_t>> Parser::parseLocCases()
{
	if (m_Lexer->isPeekToken(Token::Lb))
	{
		m_Lexer->next();

		CasesTerm<Loc_t>::Cases_t cases;
		bool isCaseRemaining = true;
		while (isCaseRemaining)
		{
			if (auto caseOpt = parseLocCase())
			{
				auto &[val, term] = caseOpt.value();
				cases[val] = newTerm(std::move(term));

				if (m_Lexer->isPeekToken(Token::Comma))
				{
					m_Lexer->next();
					isCaseRemaining = true;
				}
				else
				{
					isCaseRemaining = false;
				}
			}
			else
			{
				parseError("Expected a case mapping for cases", *m_Lexer);
			}
		}

		if (m_Lexer->isPeekToken(Token::Rb))
		{
			m_Lexer->next();

			if (m_Lexer->isPeekToken(Token::Dot))
			{
				m_Lexer->next();

				if (auto bodyOpt = parseTerm())
				{
					return CasesTerm<Loc_t>(
						std::move(cases), std::move(bodyOpt.value())
					);
				}
			}

			return CasesTerm<Loc_t>(
				std::move(cases)
			);
		}
		else
		{
			parseError("Expected closing ')' for cases", *m_Lexer);
		}
	}

	return std::nullopt;
}