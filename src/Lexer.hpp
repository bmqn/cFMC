#pragma once

#include <iostream>
#include <istream>
#include <string>
#include <optional>
#include <utility>
#include <memory>

enum class Token
{
	Nothing, // Represents nothing

	Lb, Rb, // ()
	Lab, Rab, // <>
	Lsb, Rsb, // []
	
	Astx, // *
	Dot, // .
	Eql, // =
	Comma, // ,
	Arrow, // ->

	Val, // Value, i.e. integer, bool, etc.
	Id, // Identifier, i.e. location name, variable name, function name etc.

	Eof // End of line
};

class Lexer
{
public:
	explicit Lexer(const std::string &buffer);

	Lexer(const Lexer &) = delete;
	Lexer &operator=(const Lexer &) = delete;

	Lexer(Lexer &&) = delete;
	Lexer &operator=(Lexer &&) = delete;

	const std::string &getSource() const;

	const std::string &getBuffer() const;

	Token getToken() const;
	const std::string &getTokenBuffer() const;

	void advance();

private:
	std::optional<std::pair<Token, std::string>> nextToken();

private:
	std::string m_Source;
	std::unique_ptr<std::istream> m_Stream;

	std::string m_Buffer;

	Token m_PrevToken;
	std::string m_PrevTokenBuffer;

	Token m_Token;
	std::string m_TokenBuffer;
};
