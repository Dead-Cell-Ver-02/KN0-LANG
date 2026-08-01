#pragma once

#include "Tokens.h"
#include <memory>
#include <string>
#include <vector>

class Expr
{
public:
	virtual ~Expr() = default;
};

class NumberLiteral : public Expr
{
public:
	int value;

	explicit NumberLiteral( int value )
		: value( value ) {}
};

class Binary : public Expr
{
public:
	std::unique_ptr<Expr> left;
	Token op;
	std::unique_ptr<Expr> right;

	Binary( std::unique_ptr<Expr> left ,
			const Token& op ,
			std::unique_ptr<Expr> right )
		: left( std::move( left ) ) ,
		op( op ) ,
		right( std::move( right ) )
	{}
};

class Variable : public Expr
{
public:
	std::string name;

	explicit Variable( const std::string& name )
		: name( name ) {}
};

class FunctionCall : public Expr
{
public:
	std::string name;
	std::vector<std::unique_ptr<Expr>> arguments;

	FunctionCall(
		const std::string& name ,
		std::vector<std::unique_ptr<Expr>> arguments )
		: name( name ) ,
		arguments( std::move( arguments ) )
	{}
};