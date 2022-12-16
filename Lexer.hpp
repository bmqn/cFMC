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
	
	Nil, // Sequential nil
	Dot, // Term seperator
	Id, // Identifier, i.e. location name, variable name, function name etc.

	StaticFuncDec, // Static function declaraton

	True, // Boolean true
	False, // Boolean false

	IfFunc, // If function
	PrintFunc, // Print function

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

	Token getToken() const;
	std::string getTokenText() const;

	std::string getCompleteBuffer() const;

	void advance();

private:
	std::optional<std::pair<Token, std::string>> nextToken();

private:
	std::istream *m_Input;
	
	std::string m_Buffer;
	Token m_CurrToken;

	std::string m_CompleteBuffer;
};
