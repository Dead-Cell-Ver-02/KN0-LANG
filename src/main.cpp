#include "Compiler.h"
#include "Lexer.h"
#include "Machine.h"
#include "Parser.h"
#include <iostream>

int main() {
	std::string source = R"(
		fun bump(n) {
		  let x = n + 1;
		  if (x > 5) {
		    update x to x * 2;
		  };
		  return x;
		}
		let a = invoke bump(5);
		let b = invoke bump(a);
		update b to b + invoke bump(1);
		if (b > 20) {
		  update b to b - a;
		};
		b;
		)";

	Lexer lexer( source );
	auto tokens = lexer.scanTokens();

	Parser parser( tokens , false ); // shouldLog off for cleaner output
	auto statements = parser.parse();

	Compiler compiler( false );
	CompileResult compiled = compiler.compile( statements );

	Machine machine;
	std::vector<int> finalStack = machine.run( compiled.instructions );

	std::cout << "Final stack contents:\n";
	for ( int value : finalStack ) {
		std::cout << value << "\n";
	}

	return 0;
}