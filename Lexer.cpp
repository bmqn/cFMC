#include "Lexer.hpp"

#include <iostream>

static void lexError(std::string message, const Lexer &lexer)
{
	std::string buf = lexer.getFullBuffer();
	int tokenLen = lexer.getTokenBuffer().size();

	std::string underlineErr;
	underlineErr += std::string(buf.size() - tokenLen, ' ');
	underlineErr += std::string("~");

	std::cerr << "[Lex Error] " << message << std::endl;
	std::cerr << "    " << buf << std::endl;
	std::cerr << "    " << underlineErr << std::endl;

	std::exit(1);
}

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
	case Token::Astx:
		os << '*';
		break;
	case Token::Dot:
		os << '.';
		break;
	case Token::SemiColon:
		os << ';';
		break;
	case Token::Eql:
		os << '=';
		break;
	case Token::Id:
		os << "Id";
		break;
	// case Token::IfFunc:
	// 	os << "IfFunc";
	// 	break;
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

std::string Lexer::getFullBuffer() const
{
	return m_FullBuffer;
}

std::string Lexer::getTokenBuffer() const
{
	return m_TokenBuffer;
}

Token Lexer::getToken() const
{
	return m_CurrToken;
}

void Lexer::advance()
{
	if (m_CurrToken != Token::Eof)
	{
		if (auto nextOpt = nextToken())
		{
			auto [token, buffer] = nextOpt.value();
			m_CurrToken = token;
			m_TokenBuffer = buffer;
		}
		else
		{
			lexError("Could not advance the lexer !", *this);
		}
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

	m_FullBuffer += c;

	// Eat whitespace (includes nl & cr)
	while (std::isspace(c))
	{
		c = m_Input->get();
		m_FullBuffer += c;
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
		return std::make_pair(Token::Astx, buffer);
	}
	else if (c == '.')
	{
		return std::make_pair(Token::Dot, buffer);
	}
	else if (c == ';')
	{
		return std::make_pair(Token::SemiColon, buffer);
	}
	else if (c == '=')
	{
		return std::make_pair(Token::Eql, buffer);
	}
	
	if (std::isalpha(c))
	{
		c = m_Input->get();
		m_FullBuffer += c;

		while (std::isalpha(c))
		{
			buffer += c;
			c = m_Input->get();
			m_FullBuffer += c;
		}
		m_Input->putback(c);
		m_FullBuffer.pop_back();

		if (buffer == "true")
		{
			return std::make_pair(Token::True, buffer);
		}
		else if (buffer == "false")
		{
			return std::make_pair(Token::False, buffer);
		}
		// else if (buffer == "if")
		// {
		// 	return std::make_pair(Token::IfFunc, buffer);
		// }
		else
		{
			return std::make_pair(Token::Id, buffer);
		}
	}

	return std::nullopt;
}