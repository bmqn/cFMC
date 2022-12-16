#include "Lexer.hpp"

#include <iostream>

std::ostream &operator<<(std::ostream& os, const Token &token)
{
	switch (token)
	{
	case Token::Lab:
		os << '<';
		break;
	case Token::Rab:
		os << '>';
		break;
	case Token::Lsb:
		os << '[';
		break;
	case Token::Rsb:
		os << ']';
		break;
	case Token::Nil:
		os << '*';
		break;
	case Token::Dot:
		os << '.';
		break;
	case Token::Id:
		os << "Id";
		break;
	case Token::StaticFuncDec:
		os << "StaticFuncDec";
		break;
	case Token::PrintFunc:
		os << "PrintFunc";
		break;
	case Token::IfFunc:
		os << "IfFunc";
		break;
	case Token::Eof:
		os << "Eof";
		break;
	}

	return os;
}

Lexer::Lexer(std::istream &is)
	: m_Input(&is)
{
	advance();
}

Token Lexer::getToken() const
{
	return m_CurrToken;
}

std::string Lexer::getTokenText() const
{
	return m_Buffer;
}

std::string Lexer::getCompleteBuffer() const
{
	return m_CompleteBuffer;
}

void Lexer::advance()
{
	if (auto nextOpt = nextToken())
	{
		auto [token, buffer] = nextOpt.value();
		m_CurrToken = token;
		m_Buffer = buffer;
	}
	else
	{
		std::cerr << "[Lexing Error] Could not advance the lexer !" << std::endl;
	}
}

std::optional<std::pair<Token, std::string>> Lexer::nextToken()
{
	std::string buffer;
	char c = m_Input->get();

	// End of stream
	if (c == std::char_traits<char>::eof())
	{
		return std::make_pair(Token::Eof, buffer);
	}

	m_CompleteBuffer += c;

	// Eat whitespace
	while (std::isspace(c))
	{
		c = m_Input->get();
		m_CompleteBuffer += c;
	}
	buffer += c;

	if (c == '<')
	{
		return std::make_pair(Token::Lab, buffer);
	}
	else if (c == '>')
	{
		return std::make_pair(Token::Rab, buffer);
	}
	if (c == '[')
	{
		return std::make_pair(Token::Lsb, buffer);
	}
	else if (c == ']')
	{
		return std::make_pair(Token::Rsb, buffer);
	}
	else if (c == '*')
	{
		return std::make_pair(Token::Nil, buffer);
	}
	else if (c == '.')
	{
		return std::make_pair(Token::Dot, buffer);
	}
	else if (c == 'S')
	{
		c = m_Input->get();
		m_CompleteBuffer += c;

		if (c == '=')
		{
			buffer += c;

			return std::make_pair(Token::StaticFuncDec, buffer);
		}

		m_Input->putback(c);
	}
	
	if (std::isalpha(c))
	{
		c = m_Input->get();
		m_CompleteBuffer += c;

		while (std::isalpha(c))
		{
			buffer += c;
			c = m_Input->get();
			m_CompleteBuffer += c;
		}
		m_Input->putback(c);
		m_CompleteBuffer.pop_back();

		if (buffer == "true")
		{
			return std::make_pair(Token::True, buffer);
		}
		else if (buffer == "false")
		{
			return std::make_pair(Token::False, buffer);
		}
		else if (buffer == "if")
		{
			return std::make_pair(Token::IfFunc, buffer);
		}
		else if (buffer == "print")
		{
			return std::make_pair(Token::PrintFunc, buffer);
		}
		else
		{
			return std::make_pair(Token::Id, buffer);
		}
	}

	return std::nullopt;
}