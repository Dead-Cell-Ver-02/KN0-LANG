#pragma once

#include "Expr.h"
#include <memory>
#include <string>
#include <vector>

class Stmt
{
public:
	virtual ~Stmt() = default;
};

class VarDeclaration : public Stmt
{
public:
	std::string name;
	std::unique_ptr<Expr> initializer;

	VarDeclaration(
		const std::string& name ,
		std::unique_ptr<Expr> initializer )
		: name( name ) ,
		initializer( std::move( initializer ) )
	{}
};

class ExpressionStmt : public Stmt
{
public:
	std::unique_ptr<Expr> expression;

	explicit ExpressionStmt( std::unique_ptr<Expr> expression )
		: expression( std::move( expression ) )
	{}
};

class VarUpdate : public Stmt
{
public:
	std::string name;
	std::unique_ptr<Expr> value;

	VarUpdate(
		const std::string& name ,
		std::unique_ptr<Expr> value )
		: name( name ) ,
		value( std::move( value ) )
	{}
};

class IfStmt : public Stmt
{
public:
	std::unique_ptr<Expr> condition;
	std::vector<std::unique_ptr<Stmt>> body;

	IfStmt(
		std::unique_ptr<Expr> condition ,
		std::vector<std::unique_ptr<Stmt>> body )
		: condition( std::move( condition ) ) ,
		body( std::move( body ) )
	{}
};

class FunctionDeclaration : public Stmt
{
public:
	std::string name;
	std::vector<std::string> parameters;
	std::vector<std::unique_ptr<Stmt>> body;

	FunctionDeclaration(
		const std::string& name ,
		std::vector<std::string> parameters ,
		std::vector<std::unique_ptr<Stmt>> body )
		: name( name ) ,
		parameters( std::move( parameters ) ) ,
		body( std::move( body ) )
	{}
};

class ReturnStmt : public Stmt
{
public:
	std::unique_ptr<Expr> value;

	explicit ReturnStmt( std::unique_ptr<Expr> value )
		: value( std::move( value ) )
	{}
};