//	E -> E + T
//  E -> E - T
//	E -> T
//
//  T -> T * F
//  T -> T / F
//  T -> F
//
//  F -> INTEGER
//  F -> ( E )

#pragma once
#include <string>

enum class TokenType {
	NUMBER ,
	PLUS ,
	MINUS ,
	STAR ,
	SLASH ,
	OPEN_PARENTHESIS ,
	CLOSE_PARENTHESIS ,
	END_OF_FILE ,
	LET ,
	IDENTIFIER ,
	EQUAL ,
	SEMICOLON ,
	UPDATE ,
	TO ,
	LESS_THAN ,
	GREATER_THAN ,
	OPEN_BRACE ,
	CLOSE_BRACE ,
	IF ,
	FUN ,
	COMMA ,
	RETURN ,
	INVOKE
};

struct Token {
	TokenType type;
	std::string literal = "";
};