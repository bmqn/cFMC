#include "Lexer.hpp"

#include <iostream>
#include <sstream>

static void lexError(std::string message, const Lexer &lexer)
{
	const std::string &buf = lexer.getBuffer();
	const std::string &tokenbuf = lexer.getTokenBuffer();
	int tokenLen = tokenbuf.size();

	std::cerr << "[Lex Error]" << std::endl;

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

Lexer::Lexer(const std::string &buffer)
	: m_Stream(nullptr)
	, m_PrevToken(Token::Nothing)
	, m_Token(Token::Nothing)
{
	m_Source = buffer;
	m_Stream = std::make_unique<std::istringstream>(m_Source);

	advance();
}

const std::string &Lexer::getSource() const
{
	return m_Source;
}

const std::string &Lexer::getBuffer() const
{
	return m_Buffer;
}

Token Lexer::getToken() const
{
	return m_Token;
}

const std::string &Lexer::getTokenBuffer() const
{
	return m_TokenBuffer;
}

void Lexer::advance()
{
	if (m_Token != Token::Eof)
	{
		if (auto nextOpt = nextToken())
		{
			m_PrevToken = m_Token;
			m_PrevTokenBuffer = m_TokenBuffer;

			auto [token, buffer] = nextOpt.value();

			m_Token = token;
			m_TokenBuffer = buffer;
		}
		else
		{
			lexError("Unrecognised token !", *this);
		}
	}
}

std::optional<std::pair<Token, std::string>> Lexer::nextToken()
{
	std::string buffer;

	char c = m_Stream->get();
	if (c == std::istream::traits_type::eof())
	{
		return std::make_pair(Token::Eof, buffer);
	}
	m_Buffer += c;
	
	// Eat whitespace (includes nl & cr)
	while (std::isspace(c))
	{
		c = m_Stream->get();
		if (c == std::istream::traits_type::eof())
		{
			return std::make_pair(Token::Eof, buffer);
		}
		m_Buffer += c;
	}

	buffer += c;

	if (c == '(')
	{
		return std::make_pair(Token::Lb, buffer);
	}
	else if (c == ')')
	{
		return std::make_pair(Token::Rb, buffer);
	}
	else if (c == '<')
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
	else if (c == '=')
	{
		return std::make_pair(Token::Eql, buffer);
	}
	else if (c == ',')
	{
		return std::make_pair(Token::Comma, buffer);
	}
	else if (c == '-')
	{
		c = m_Stream->get();
		m_Buffer += c;

		if (c == '>')
		{
			return std::make_pair(Token::Arrow, buffer);
		}

		m_Stream->putback(c);
		m_Buffer.pop_back();
	}
	else if (std::isalpha(c))
	{
		c = m_Stream->get();
		m_Buffer += c;

		while (std::isalnum(c) || c == '_')
		{
			buffer += c;
			c = m_Stream->get();
			m_Buffer += c;
		}
		m_Stream->putback(c);
		m_Buffer.pop_back();

		return std::make_pair(Token::Id, buffer);
	}
	else if (std::isdigit(c))
	{
		c = m_Stream->get();
		m_Buffer += c;

		while (std::isdigit(c))
		{
			buffer += c;
			c = m_Stream->get();
			m_Buffer += c;
		}
		m_Stream->putback(c);
		m_Buffer.pop_back();

		return std::make_pair(Token::Val, buffer);
	}

	return std::nullopt;
}