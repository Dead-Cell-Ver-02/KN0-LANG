#include "Lexer.h"
#include <iostream>

int main() {
	Lexer lexer( "let x = 5 + 3;" );
	auto tokens = lexer.scanTokens();

	for ( const auto& token : tokens ) {
		std::cout << "Type: " << static_cast< int >( token.type )
			<< ", Literal: " << token.literal << "\n";
	}

	return 0;
}