#include "Lexer.h"
#include <cctype>
#include <iostream>
#include <unordered_map>

Lexer::Lexer( const std::string& input ) : input( input )
{

}

std::vector<Token> Lexer::scanTokens()
{
	while ( !isAtEnd() )
	{
		scanNextToken();
	}
	addToken( TokenType::END_OF_FILE );
	return tokens;
}



char Lexer::advance()
{
	char returnCharacter = input[ currentPosition ];
	currentPosition++;
	return returnCharacter;
}

bool Lexer::isAtEnd()
{
	return currentPosition >= input.length();
}

void Lexer::addToken( TokenType type )
{
	tokens.push_back( Token( type ) );
}

char Lexer::peek()
{
	if ( isAtEnd() )
		return '\0';
	return input[ currentPosition ];
}

void Lexer::scanNumber()
{
	int start = currentPosition - 1;

	while ( std::isdigit( static_cast< unsigned char >( peek() ) ) )
	{
		advance();
	}

	std::string number = input.substr( start , currentPosition - start );
	tokens.emplace_back( TokenType::NUMBER , number );
}

void Lexer::scanIdentifier()
{
	int start = currentPosition - 1;

	while ( std::isalpha( static_cast< unsigned char >( peek() ) ) )
	{
		advance();
	}

	std::string literal = input.substr( start , currentPosition - start );

	static const std::unordered_map<std::string , TokenType> keywords =
	{
		{"fun", TokenType::FUN},
		{"invoke", TokenType::INVOKE},
		{"return", TokenType::RETURN},
		{"if", TokenType::IF},
		{"let", TokenType::LET},
		{"update", TokenType::UPDATE},
		{"to", TokenType::TO}
	};

	auto it = keywords.find( literal );

	if ( it != keywords.end() )
	{
		tokens.emplace_back( it->second );
	}
	else
	{
		tokens.emplace_back( TokenType::IDENTIFIER , literal );
	}
}

void Lexer::scanNextToken()
{
	char currentCharacter = advance();

	switch ( currentCharacter )
	{
	case ',':
		addToken( TokenType::COMMA );
		break;

	case '{':
		addToken( TokenType::OPEN_BRACE );
		break;

	case '}':
		addToken( TokenType::CLOSE_BRACE );
		break;

	case '=':
		addToken( TokenType::EQUAL );
		break;

	case ';':
		addToken( TokenType::SEMICOLON );
		break;

	case '(':
		addToken( TokenType::OPEN_PARENTHESIS );
		break;

	case ')':
		addToken( TokenType::CLOSE_PARENTHESIS );
		break;

	case '+':
		addToken( TokenType::PLUS );
		break;

	case '-':
		addToken( TokenType::MINUS );
		break;

	case '*':
		addToken( TokenType::STAR );
		break;

	case '/':
		addToken( TokenType::SLASH );
		break;

	case '<':
		addToken( TokenType::LESS_THAN );
		break;

	case '>':
		addToken( TokenType::GREATER_THAN );
		break;

	case ' ':
	case '\n':
	case '\t':
	case '\r':
		// Ignore whitespace
		break;

	default:
		if ( std::isdigit( static_cast< unsigned char >( currentCharacter ) ) )
		{
			scanNumber();
		}
		else if ( std::isalpha( static_cast< unsigned char >( currentCharacter ) ) )
		{
			scanIdentifier();
		}
		else
		{
			throw std::runtime_error(
				"Unexpected token: " + std::string( 1 , currentCharacter )
			);
		}
	}
}
