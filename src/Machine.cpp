// Machine.cpp
#include "Machine.h"
#include <stdexcept>
#include <string>

std::vector<int> Machine::run( const std::vector<std::unique_ptr<Instruction>>& instructions ) {
	stack.clear();
	callStack.clear();

	std::vector<std::optional<int>> activeLocals;
	int instructionPointer = 0;

	while ( instructionPointer < static_cast< int >( instructions.size() ) ) {
		auto next = execute( *instructions[ instructionPointer ] , activeLocals , instructionPointer + 1 );
		instructionPointer = next.has_value() ? *next : instructionPointer + 1;
	}

	return stack;
}

std::optional<int> Machine::execute(
	const Instruction& instruction ,
	std::vector<std::optional<int>>& activeLocals ,
	int nextInstructionPointer
) {
	if ( auto* i = dynamic_cast< const PushInt* >( &instruction ) ) {
		stack.push_back( i->value );
	}
	else if ( dynamic_cast< const Add* >( &instruction ) ) {
		int right = pop(); int left = pop();
		stack.push_back( left + right );
	}
	else if ( dynamic_cast< const Sub* >( &instruction ) ) {
		int right = pop(); int left = pop();
		stack.push_back( left - right );
	}
	else if ( dynamic_cast< const Mul* >( &instruction ) ) {
		int right = pop(); int left = pop();
		stack.push_back( left * right );
	}
	else if ( dynamic_cast< const Div* >( &instruction ) ) {
		int right = pop(); int left = pop();
		stack.push_back( left / right );
	}
	else if ( dynamic_cast< const LessThan* >( &instruction ) ) {
		int right = pop(); int left = pop();
		stack.push_back( left < right ? 1 : 0 );
	}
	else if ( dynamic_cast< const GreaterThan* >( &instruction ) ) {
		int right = pop(); int left = pop();
		stack.push_back( left > right ? 1 : 0 );
	}
	else if ( auto* i = dynamic_cast< const StoreLocal* >( &instruction ) ) {
		storeLocal( activeLocals , i->slot , pop() );
	}
	else if ( auto* i = dynamic_cast< const LoadLocal* >( &instruction ) ) {
		stack.push_back( loadLocal( activeLocals , i->slot ) );
	}
	else if ( auto* i = dynamic_cast< const Jump* >( &instruction ) ) {
		return i->target;
	}
	else if ( auto* i = dynamic_cast< const JumpIfFalse* >( &instruction ) ) {
		int condition = pop();
		if ( condition == 0 ) {
			return i->target;
		}
	}
	else if ( auto* i = dynamic_cast< const CallFunction* >( &instruction ) ) {
		auto callLocals = createFunctionLocals( i->arity );
		callStack.push_back( CallFrame { nextInstructionPointer, activeLocals } );
		activeLocals = std::move( callLocals );
		return i->address;
	}
	else if ( dynamic_cast< const Return* >( &instruction ) ) {
		if ( callStack.empty() ) {
			return std::nullopt; // signals "halt" in Kotlin; see note below
		}
		CallFrame frame = std::move( callStack.back() );
		callStack.pop_back();
		activeLocals = std::move( frame.locals );
		return frame.returnAddress;
	}

	return std::nullopt;
}

int Machine::pop() {
	if ( stack.empty() ) {
		throw std::runtime_error( "Stack underflow" );
	}
	int value = stack.back();
	stack.pop_back();
	return value;
}

void Machine::storeLocal( std::vector<std::optional<int>>& locals , int slot , int value ) {
	while ( static_cast< int >( locals.size() ) <= slot ) {
		locals.push_back( std::nullopt );
	}
	locals[ slot ] = value;
}

int Machine::loadLocal( std::vector<std::optional<int>>& locals , int slot ) {
	if ( slot >= static_cast< int >( locals.size() ) || !locals[ slot ].has_value() ) {
		throw std::runtime_error( "Undefined local slot " + std::to_string( slot ) );
	}
	return *locals[ slot ];
}

std::vector<std::optional<int>> Machine::createFunctionLocals( int arity ) {
	std::vector<int> arguments;
	for ( int i = 0; i < arity; ++i ) {
		arguments.push_back( pop() );
	}
	std::reverse( arguments.begin() , arguments.end() );

	std::vector<std::optional<int>> callLocals;
	for ( size_t i = 0; i < arguments.size(); ++i ) {
		storeLocal( callLocals , static_cast< int >( i ) , arguments[ i ] );
	}
	return callLocals;
}