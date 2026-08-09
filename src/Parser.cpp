#include "Parser.h"
#include <iostream>

Parser::Parser( const std::vector<Token>& tokens , bool shouldLog )
	: tokens( tokens ) , shouldLog( shouldLog )
{}

void Parser::log( const std::string& message )
{
	if ( shouldLog )
		std::cout << message << "\n";
}

std::vector<std::unique_ptr<Stmt>> Parser::parse()
{
	log( "parse() Top level public parse function called" );

	std::vector<std::unique_ptr<Stmt>> statements;

	while ( !isAtEnd() )
		statements.push_back( parseTopLevelStatement() );

	log( "parse() done parsing expression, verifying EOF exists" );

	consume( TokenType::END_OF_FILE ,
			 "Expected EOF to terminate the program" );

	return statements;
}

std::unique_ptr<Stmt> Parser::parseTopLevelStatement()
{
	if ( match( { TokenType::FUN } ) )
		return parseFunctionDeclaration();

	return parseStatement();
}

std::unique_ptr<Stmt> Parser::parseFunctionDeclaration()
{
	std::string name = consume( TokenType::IDENTIFIER , "Function Needs a Name" ).literal;
	consume( TokenType::OPEN_PARENTHESIS , "Expected ( in function declaration" );

	std::vector<std::string> parameters;
	std::vector<std::unique_ptr<Stmt>> body;

	while ( !check( TokenType::CLOSE_PARENTHESIS ) )
	{
		parameters.push_back( consume( TokenType::IDENTIFIER , "Expected Identifier" ).literal );

		while ( match( { TokenType::COMMA } ) )
			parameters.push_back( consume( TokenType::IDENTIFIER , "Expected Identifier" ).literal );
	}

	consume( TokenType::CLOSE_PARENTHESIS , "Expected ) in function definition" );
	consume( TokenType::OPEN_BRACE , "Expected {" );

	while ( !check( TokenType::CLOSE_BRACE ) && !isAtEnd() )
		body.push_back( parseStatement() );

	consume( TokenType::CLOSE_BRACE , "Expected }" );

	return std::make_unique<FunctionDeclaration>(
		name ,
		std::move( parameters ) ,
		std::move( body )
	);
}

std::unique_ptr<Stmt> Parser::parseReturnStatement()
{
	std::unique_ptr<Expr> value = parseCExpression();
	consume( TokenType::SEMICOLON , "Expected ;" );
	return std::make_unique<ReturnStmt>( std::move( value ) );
}

std::unique_ptr<Stmt> Parser::parseStatement()
{
	switch ( peek().type )
	{
	case TokenType::RETURN:
		advance();
		return parseReturnStatement();

	case TokenType::LET:
		advance();
		return parseVarDeclaration();

	case TokenType::UPDATE:
		advance();
		return parseVarUpdate();

	case TokenType::IF:
		advance();
		return parseIfStatement();

	default:
		return parseExpressionStatement();
	}
}

std::unique_ptr<Stmt> Parser::parseIfStatement()
{
	consume( TokenType::OPEN_PARENTHESIS , "Expected ( after if" );
	std::unique_ptr<Expr> condition = parseCExpression();
	consume( TokenType::CLOSE_PARENTHESIS , "Expected ) after if" );

	std::vector<std::unique_ptr<Stmt>> body;
	consume( TokenType::OPEN_BRACE , "Expected { after )" );
	while ( !check( TokenType::CLOSE_BRACE ) && !isAtEnd() )
		body.push_back( parseStatement() );
	consume( TokenType::CLOSE_BRACE , "Expected } to end if statement body" );
	consume( TokenType::SEMICOLON , "Expected ; to end if statement" );

	return std::make_unique<IfStmt>(
		std::move( condition ) ,
		std::move( body )
	);
}

std::unique_ptr<Stmt> Parser::parseVarUpdate()
{
	std::string name = consume( TokenType::IDENTIFIER , "must specify variable anme to update" ).literal;
	consume( TokenType::TO , "Expected literal to" );
	std::unique_ptr<Expr> value = parseExpression();
	consume( TokenType::SEMICOLON , "Expected : to end statement" );

	return std::make_unique<VarUpdate>(
		name ,
		std::move( value )
	);
}

std::unique_ptr<Stmt> Parser::parseVarDeclaration()
{
	std::string name = consume( TokenType::IDENTIFIER , "Expected name for variable declaration" ).literal;
	consume( TokenType::EQUAL , "Expected = sign after variable name" );
	std::unique_ptr<Expr> initializer = parseExpression();
	consume( TokenType::SEMICOLON , "Expected ; after variable declaration" );

	return std::make_unique<VarDeclaration>(
		name ,
		std::move( initializer )
	);
}

std::unique_ptr<Stmt> Parser::parseExpressionStatement()
{
	std::unique_ptr<Expr> expression = parseCExpression();
	consume( TokenType::SEMICOLON , "Expected ; after expression statement" );

	return std::make_unique<ExpressionStmt>(
		std::move( expression )
	);
}

std::unique_ptr<Expr> Parser::parseCExpression()
{
	auto workingExpression = parseExpression();

	if ( match( { TokenType::LESS_THAN, TokenType::GREATER_THAN } ) )
	{
		Token op = previous();
		auto parsedExpression = parseExpression();

		workingExpression = std::make_unique<Binary>(
			std::move( workingExpression ) ,
			op ,
			std::move( parsedExpression )
		);
	}

	return workingExpression;
}

std::unique_ptr<Expr> Parser::parseExpression()
{
	log( "parseExpression() called, will start by parsing term()" );

	auto workingExpression = parseTerm();

	log( "parseExpression() done parsing term, now checking for + - Term" );

	while ( match( { TokenType::PLUS, TokenType::MINUS } ) )
	{
		log( "parseExpression() found a " + previous().literal + ", will parse term again" );

		const Token& op = previous();
		auto parsedTerm = parseTerm();

		// Left-associative
		workingExpression = std::make_unique<Binary>(
			std::move( workingExpression ) ,
			op ,
			std::move( parsedTerm )
		);
	}

	return workingExpression;
}

std::unique_ptr<Expr> Parser::parseTerm()
{
	log( "parseTerm() called, will start by calling parseFactor()" );

	auto workingExpression = parseFactor();

	log( "parseTerm() done parsing term, now checking for * / Term" );

	while ( match( { TokenType::STAR, TokenType::SLASH } ) )
	{
		log( "parseTerm() found an operator, will parse factor again" );

		const Token& op = previous();
		auto parsedFactor = parseFactor();

		workingExpression = std::make_unique<Binary>(
			std::move( workingExpression ) ,
			op ,
			std::move( parsedFactor )
		);
	}

	return workingExpression;
}

std::unique_ptr<Expr> Parser::parseFactor()
{
	log( "parseFactor() called" );

	// Case 1: Number
	if ( match( { TokenType::NUMBER } ) )
	{
		log( "Matched a number" );

		return std::make_unique<NumberLiteral>(
			std::stoi( previous().literal )
		);
	}

	// Case 2: ( expression )
	if ( match( { TokenType::OPEN_PARENTHESIS } ) )
	{
		log( "Matched '('" );

		auto parsedExpression = parseExpression();

		consume(
			TokenType::CLOSE_PARENTHESIS ,
			"Expected ')' following '('"
		);

		return parsedExpression;
	}

	// Case 3: Identifier
	if ( match( { TokenType::IDENTIFIER } ) )
	{
		return std::make_unique<Variable>(
			previous().literal
		);
	}

	// Case 4: Function invocation
	if ( match( { TokenType::INVOKE } ) )
	{
		std::string name =
			consume( TokenType::IDENTIFIER ,
					 "Expected identifier" ).literal;

		consume(
			TokenType::OPEN_PARENTHESIS ,
			"Expected '('"
		);

		std::vector<std::unique_ptr<Expr>> args;

		while ( !check( TokenType::CLOSE_PARENTHESIS ) )
		{
			args.push_back( parseCExpression() );

			while ( match( { TokenType::COMMA } ) )
			{
				args.push_back( parseCExpression() );
			}
		}

		consume(
			TokenType::CLOSE_PARENTHESIS ,
			"Expected ')'"
		);

		return std::make_unique<FunctionCall>(
			name ,
			std::move( args )
		);
	}

	throw std::runtime_error(
		"Unable to parse factor, expected a number or '('"
	);
}

const Token& Parser::peek() const
{
	return tokens[ currentPosition ];
}

bool Parser::isAtEnd() const
{
	return peek().type == TokenType::END_OF_FILE;
}

const Token& Parser::previous() const
{
	return tokens[ currentPosition - 1 ];
}

const Token& Parser::advance()
{
	if ( !isAtEnd() )
	{
		currentPosition++;
	}

	return previous();
}

bool Parser::check( TokenType type ) const
{
	return peek().type == type;
}

const Token& Parser::consume( TokenType type , const std::string& message )
{
	if ( check( type ) )
	{
		return advance();
	}

	throw std::runtime_error( message );
}

bool Parser::match( std::initializer_list<TokenType> types )
{
	for ( TokenType type : types )
	{
		if ( check( type ) )
		{
			advance();
			return true;
		}
	}

	return false;
}