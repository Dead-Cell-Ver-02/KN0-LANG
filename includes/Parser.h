#pragma once
#include "Expr.h"
#include "stmt.h"
#include "Tokens.h"

#include <initializer_list>
#include <vector>

class Parser
{
public:
	Parser( const std::vector<Token>& tokens , bool shouldLog );
	std::vector<std::unique_ptr<Stmt>> parse();

private:
	std::vector<Token> tokens;
	bool shouldLog;
	int currentPosition = 0;

	void log( const std::string& message );

	std::unique_ptr<Stmt> parseTopLevelStatement();
	std::unique_ptr<Stmt> parseFunctionDeclaration();
	std::unique_ptr<Stmt> parseReturnStatement();
	std::unique_ptr<Stmt> parseStatement();
	std::unique_ptr<Stmt> parseIfStatement();
	std::unique_ptr<Stmt> parseVarUpdate();
	std::unique_ptr<Stmt> parseVarDeclaration();
	std::unique_ptr<Stmt> parseExpressionStatement();

	std::unique_ptr<Expr> parseCExpression();
	std::unique_ptr<Expr> parseExpression();
	std::unique_ptr<Expr> parseTerm();
	std::unique_ptr<Expr> parseFactor();

	const Token& peek() const;
	bool isAtEnd() const;
	const Token& previous() const;
	const Token& advance();
	bool check( TokenType type ) const;
	const Token& consume( TokenType type , const std::string& message );
	bool match( std::initializer_list<TokenType> types );
};