#include "Lexer.hpp"

#include <iostream>
#include <sstream>

Lexer::Lexer(const std::string &source)
	: m_Stream(nullptr)
{
	m_Stream = std::make_unique<std::istringstream>(source);

	m_CurrCharIdx = 0;
	m_CurrTokenIdx = 0;

	for (auto i = 0; i < s_Lookahead; ++i)
	{
		if ((!m_Tokens.empty() && m_Tokens.back() != Token::Eof) ||
			m_Tokens.empty())
		{
			if (auto tokenOpt = advance())
			{
				auto [token, buffer] = tokenOpt.value();
				
				m_Tokens.push_back(token);
				m_Buffers.push_back(buffer);
			}
		}
	}
}

void Lexer::next()
{
	if ((!m_Tokens.empty() && m_Tokens.back() != Token::Eof) ||
		m_Tokens.empty())
	{
		if (auto tokenOpt = advance())
		{
			auto [token, buffer] = tokenOpt.value();

			m_Tokens.push_back(token);
			m_Buffers.push_back(buffer);
		}
	}
	
	if (m_CurrTokenIdx < m_Tokens.size())
	{
		m_CurrTokenIdx++;
	}
}

std::optional<std::pair<Token, std::string>> Lexer::getPeek(size_t n) const
{
	if (n < s_Lookahead)
	{
		auto token = m_Tokens[m_CurrTokenIdx + n];
		auto buffer = m_Buffers[m_CurrTokenIdx + n];

		return std::make_pair(token, buffer);
	}

	return std::nullopt;
}

std::optional<Token> Lexer::getPeekToken(size_t n) const
{
	if (n < s_Lookahead)
	{
		auto token = m_Tokens[m_CurrTokenIdx + n];

		return token;
	}

	return std::nullopt;
}

std::optional<std::string> Lexer::getPeekBuffer(size_t n) const
{
	if (n < s_Lookahead)
	{
		auto buffer = m_Buffers[m_CurrTokenIdx + n];

		return buffer;
	}

	return std::nullopt;
}

bool Lexer::isPeekToken(Token token, size_t n) const
{
	if (n < s_Lookahead)
	{
		return token == m_Tokens[m_CurrTokenIdx + n];;
	}

	return false;
}

const std::string &Lexer::getBuffer() const
{
	return m_Buffer;
}

std::optional<std::pair<Token, std::string>> Lexer::advance()
{
	std::string buffer;

	char c = m_Stream->get();
	
	if (c == std::istream::traits_type::eof())
	{
		return std::make_pair(Token::Eof, buffer);
	}

	m_CurrCharIdx++;
	m_Buffer += c;
	
	// Eat whitespace (includes nl & cr)
	while (std::isspace(c))
	{
		c = m_Stream->get();
		
		if (c == std::istream::traits_type::eof())
		{
			return std::make_pair(Token::Eof, buffer);
		}

		m_CurrCharIdx++;
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
		return std::make_pair(Token::Asterisk, buffer);
	}
	else if (c == '.')
	{
		return std::make_pair(Token::Dot, buffer);
	}
	else if (c == '=')
	{
		return std::make_pair(Token::Equal, buffer);
	}
	else if (c == ',')
	{
		return std::make_pair(Token::Comma, buffer);
	}
	else if (c == '_')
	{
		return std::make_pair(Token::Underscore, buffer);
	}
	else if (c == '@')
	{
		return std::make_pair(Token::Ampersand, buffer);
	}
	else if (c == '#')
	{
		return std::make_pair(Token::Hash, buffer);
	}
	else if (c == '+')
	{
		return std::make_pair(Token::Plus, buffer);
	}
	else if (c == '-')
	{
		c = m_Stream->get();
		m_CurrCharIdx++;
		m_Buffer += c;

		if (c == '>')
		{
			return std::make_pair(Token::Arrow, buffer);
		}

		m_Stream->putback(c);
		m_CurrCharIdx--;
		m_Buffer.pop_back();

		return std::make_pair(Token::Minus, buffer);
	}
	else if (std::isalpha(c))
	{
		c = m_Stream->get();
		m_CurrCharIdx++;
		m_Buffer += c;

		while (std::isalnum(c) || c == '_')
		{
			buffer += c;

			c = m_Stream->get();
			m_CurrCharIdx++;
			m_Buffer += c;
		}

		m_Stream->putback(c);
		m_CurrCharIdx--;
		m_Buffer.pop_back();

		return std::make_pair(Token::Id, buffer);
	}
	else if (std::isdigit(c))
	{
		c = m_Stream->get();
		m_CurrCharIdx++;
		m_Buffer += c;

		while (std::isdigit(c))
		{
			buffer += c;
			c = m_Stream->get();
			m_CurrCharIdx++;
		}
		m_Stream->putback(c);
		m_CurrCharIdx--;
		m_Buffer.pop_back();

		return std::make_pair(Token::Primitive, buffer);
	}

	return std::nullopt;
}