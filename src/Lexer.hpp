#pragma once

#include <iostream>
#include <istream>
#include <string>
#include <optional>
#include <utility>
#include <memory>
#include <vector>

enum class Token
{
	Lb, Rb, // ( )
	Lab, Rab, // < >
	Lsb, Rsb, // [ ]
	Asterisk, // *
	Dot, // .
	Equal, // =
	Comma, // ,
	Underscore, // _
	Ampersand, // @
	Hash, // #

	Arrow, // ->

	Primitive, // Primitive, i.e. integer, bool, etc.
	Id, // Identifier, i.e. location name, variable name, function name etc.

	Eof // End of line
};

class Lexer
{
public:
	explicit Lexer(const std::string &source);

	Lexer(const Lexer &) = delete;
	Lexer &operator=(const Lexer &) = delete;

	Lexer(Lexer &&) = delete;
	Lexer &operator=(Lexer &&) = delete;

	const std::string &getBuffer() const;

	void next();
	std::optional<std::pair<Token, std::string>> getPeek(size_t n = 0) const;
	std::optional<Token> getPeekToken(size_t n = 0) const;
	std::optional<std::string> getPeekBuffer(size_t n = 0) const;
	bool isPeekToken(Token token, size_t n = 0) const;

private:
	std::optional<std::pair<Token, std::string>> advance();

private:
	static const size_t s_Lookahead = 3;

	size_t m_CurrCharIdx;
	std::string m_Buffer;

	size_t m_CurrTokenIdx;
	std::vector<Token> m_Tokens;
	std::vector<std::string> m_Buffers;

	std::unique_ptr<std::istream> m_Stream;
};
