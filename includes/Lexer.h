#pragma once
#include "Tokens.h"
#include <string>
#include <vector>

class Lexer
{
public:
	Lexer( const std::string& input );
	~Lexer() = default;

	std::vector<Token> scanTokens();

private:
	std::string input;
	std::vector<Token> tokens;
	int currentPosition = 0;

	char advance();
	bool isAtEnd();
	void addToken( TokenType type );
	void scanNextToken();
	void scanNumber();
	char peek();
	void scanIdentifier();
};