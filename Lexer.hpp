#pragma once

#include <iostream>
#include <istream>
#include <string>
#include <optional>
#include <utility>

enum class Token
{
	Lab, Rab, // Angle brackets
	Lsb, Rsb, // Square brackets
	
	Astx, // Sequential nil
	Dot, // Term separator
	SemiColon, // Semi-colon
	Eql, // Equal sign

	Id, // Identifier, i.e. location name, variable name, function name etc.

	True, // Boolean true
	False, // Boolean false

	// IfFunc, // If function

	Eof
};

std::ostream &operator<<(std::ostream& os, const Token &token);

class Lexer
{
public:
	explicit Lexer(std::istream &is);

	Lexer(const Lexer &) = delete;
	Lexer &operator=(const Lexer &) = delete;

	Lexer(Lexer &&) = delete;
	Lexer &operator=(Lexer &&) = delete;

	std::string getFullBuffer() const;
	std::string getTokenBuffer() const;
	Token getToken() const;

	void advance();

private:
	std::optional<std::pair<Token, std::string>> nextToken();

private:
	std::istream *m_Input;
	
	std::string m_FullBuffer;
	std::string m_TokenBuffer;
	Token m_CurrToken;
};
