#include "Compiler.h"

#include <iostream>
#include <stdexcept>
#include <utility>

Compiler::Compiler( bool shouldLog )
	: shouldLog( shouldLog )
{}

void Compiler::log( const std::string& message )
{
	if ( shouldLog )
	{
		std::cout << message << '\n';
	}
}

CompileResult Compiler::compile(
	const std::vector<std::unique_ptr<Stmt>>& statements )
{
	log( "Top level compile function called" );
	LocalContext mainContext;
	std::vector<std::unique_ptr<Instruction>> instructions;

	// Placeholder jump.
	instructions.push_back(
		std::make_unique<Jump>( 999 )
	);

	// Register function signatures.
	for ( const auto& statement : statements )
	{
		if ( const auto* function = dynamic_cast< const FunctionDeclaration* >( statement.get() ) )
		{
			registerSignature( *function );
		}
	}

	// Parse function bodies.
	for ( const auto& statement : statements )
	{
		if ( const auto* function =
			 dynamic_cast< const FunctionDeclaration* >( statement.get() ) )
		{
			setFunctionAddress( function->name , instructions.size() );

			emitFunctionDeclaration(
				*function ,
				instructions
			);
		}
	}

	// The first instruction jumps over all function bodies
	// directly to the main program.
	instructions[ 0 ] = std::make_unique<Jump>(
		static_cast< int >( instructions.size() )
	);

	// Emit top-level statements.
	for ( const auto& statement : statements )
	{
		if ( dynamic_cast< const FunctionDeclaration* >( statement.get() ) == nullptr )
		{
			emit(
				*statement ,
				instructions ,
				mainContext
			);
		}
	}

	patchFunctionCalls( instructions );

	return CompileResult {
		std::move( instructions )
	};
}


void Compiler::patchFunctionCalls(
	std::vector<std::unique_ptr<Instruction>>& instructions )
{
	for ( const PendingFunctionCall& pendingCall : pendingFunctionCalls )
	{
		auto it = functionSignatures.find( pendingCall.name );

		if ( it == functionSignatures.end() )
		{
			throw std::runtime_error( "Unreachable" );
		}

		const FunctionSignature& signature = it->second;

		instructions[ pendingCall.instructionIndex ] =
			std::make_unique<CallFunction>(
				signature.address ,
				pendingCall.arity
			);
	}
}


void Compiler::setFunctionAddress(
	const std::string& name ,
	int address )
{
	auto it = functionSignatures.find( name );

	if ( it == functionSignatures.end() )
	{
		throw std::runtime_error(
			"Function signature should already exist"
		);
	}

	it->second.address = address;
}


void Compiler::emitFunctionDeclaration(
	const FunctionDeclaration& stmt ,
	std::vector<std::unique_ptr<Instruction>>& instructions )
{
	if ( stmt.body.empty() ||
		 dynamic_cast< const ReturnStmt* >( stmt.body.back().get() ) == nullptr )
	{
		throw std::runtime_error(
			"Functions need to end with a return statement."
		);
	}

	LocalContext functionContext;
	functionContext.isFunctionBody = true;

	for ( const std::string& parameter : stmt.parameters )
	{
		if ( functionContext.locals.contains( parameter ) )
		{
			throw std::runtime_error(
				"Duplicate parameter definition"
			);
		}

		int slot = functionContext.nextLocalSlot;
		functionContext.nextLocalSlot++;

		functionContext.locals[ parameter ] = slot;
	}

	for ( const auto& bodyStatement : stmt.body )
	{
		emit(
			*bodyStatement ,
			instructions ,
			functionContext
		);
	}
}


void Compiler::registerSignature(
	const FunctionDeclaration& stmt )
{
	if ( functionSignatures.contains( stmt.name ) )
	{
		throw std::runtime_error(
			"Duplicate function declaration"
		);
	}

	std::unordered_set<std::string> seenParameters;

	for ( const std::string& parameter : stmt.parameters )
	{
		if ( !seenParameters.insert( parameter ).second )
		{
			throw std::runtime_error(
				"Duplicate parameter definition in function"
			);
		}
	}

	functionSignatures.emplace(
		stmt.name ,
		FunctionSignature { stmt.parameters }
	);
}


void Compiler::emit(
	const Stmt& stmt ,
	std::vector<std::unique_ptr<Instruction>>& instructions ,
	LocalContext& context )
{
	if ( const auto* expressionStmt =
		 dynamic_cast< const ExpressionStmt* >( &stmt ) )
	{
		emit(
			*expressionStmt->expression ,
			instructions ,
			context
		);
	}

	else if ( const auto* varDeclaration =
			  dynamic_cast< const VarDeclaration* >( &stmt ) )
	{
		if ( context.locals.contains( varDeclaration->name ) )
		{
			throw std::runtime_error(
				"Duplicate definition of variable detected: " +
				varDeclaration->name
			);
		}

		emit(
			*varDeclaration->initializer ,
			instructions ,
			context
		);

		int slot = context.nextLocalSlot;
		context.nextLocalSlot++;

		context.locals[ varDeclaration->name ] = slot;

		instructions.push_back(
			std::make_unique<StoreLocal>( slot )
		);
	}

	else if ( const auto* varUpdate =
			  dynamic_cast< const VarUpdate* >( &stmt ) )
	{
		auto it = context.locals.find( varUpdate->name );

		if ( it == context.locals.end() )
		{
			throw std::runtime_error(
				"Failed to update variable: " +
				varUpdate->name +
				" before declaration"
			);
		}

		int slot = it->second;

		emit(
			*varUpdate->value ,
			instructions ,
			context
		);

		instructions.push_back(
			std::make_unique<StoreLocal>( slot )
		);
	}

	else if ( const auto* ifStmt =
			  dynamic_cast< const IfStmt* >( &stmt ) )
	{
		// Emit condition.
		emit(
			*ifStmt->condition ,
			instructions ,
			context
		);

		// Placeholder jump target.
		int jumpInstructionIndex =
			static_cast< int >( instructions.size() );

		instructions.push_back(
			std::make_unique<JumpIfFalse>( 999 )
		);

		// Emit body.
		for ( const auto& bodyStatement : ifStmt->body )
		{
			emit(
				*bodyStatement ,
				instructions ,
				context
			);
		}

		// The target is the instruction immediately
		// after the if body.
		int realJumpLocation =
			static_cast< int >( instructions.size() );

		instructions[ jumpInstructionIndex ] =
			std::make_unique<JumpIfFalse>(
				realJumpLocation
			);
	}

	else if ( dynamic_cast< const FunctionDeclaration* >( &stmt ) )
	{
		throw std::runtime_error(
			"Functions can only be declared at the top level"
		);
	}

	else if ( const auto* returnStmt =
			  dynamic_cast< const ReturnStmt* >( &stmt ) )
	{
		if ( !context.isFunctionBody )
		{
			throw std::runtime_error(
				"Return statements may only appear "
				"within the body of a function"
			);
		}

		emit(
			*returnStmt->value ,
			instructions ,
			context
		);

		instructions.push_back(
			std::make_unique<Return>()
		);
	}
}


void Compiler::emit(
	const Expr& expr ,
	std::vector<std::unique_ptr<Instruction>>& instructions ,
	LocalContext& context )
{
	int logId = nextUniqueNumber++;

	if ( const auto* number =
		 dynamic_cast< const NumberLiteral* >( &expr ) )
	{
		log(
			"[" + std::to_string( logId ) +
			"] Emit called with number literal " +
			std::to_string( number->value )
		);

		instructions.push_back(
			std::make_unique<PushInt>( number->value )
		);
	}

	else if ( const auto* binary =
			  dynamic_cast< const Binary* >( &expr ) )
	{
		log(
			"[" + std::to_string( logId ) +
			"] Emit called with binary expression"
		);

		log(
			"[" + std::to_string( logId ) +
			"] Recursing on the left side"
		);

		emit(
			*binary->left ,
			instructions ,
			context
		);

		log(
			"[" + std::to_string( logId ) +
			"] Recursing on the right side"
		);

		emit(
			*binary->right ,
			instructions ,
			context
		);

		log(
			"[" + std::to_string( logId ) +
			"] Now adding binary operator"
		);

		instructions.push_back(
			instructionForOperator( binary->op )
		);
	}

	else if ( const auto* variable =
			  dynamic_cast< const Variable* >( &expr ) )
	{
		auto it = context.locals.find( variable->name );

		if ( it == context.locals.end() )
		{
			throw std::runtime_error(
				"Referencing undefined variable: " +
				variable->name
			);
		}

		instructions.push_back(
			std::make_unique<LoadLocal>( it->second )
		);
	}

	else if ( const auto* functionCall =
			  dynamic_cast< const FunctionCall* >( &expr ) )
	{
		auto it =
			functionSignatures.find( functionCall->name );

		if ( it == functionSignatures.end() )
		{
			throw std::runtime_error(
				"Calling unknown function: " +
				functionCall->name
			);
		}

		const FunctionSignature& signature = it->second;

		if ( signature.parameters.size() !=
			 functionCall->arguments.size() )
		{
			throw std::runtime_error(
				"Function expected a different number "
				"of arguments than it received"
			);
		}

		for ( const auto& argument : functionCall->arguments )
		{
			emit(
				*argument ,
				instructions ,
				context
			);
		}

		if ( signature.address > 0 )
		{
			instructions.push_back(
				std::make_unique<CallFunction>(
					signature.address ,
					static_cast< int >(
						functionCall->arguments.size()
						)
				)
			);
		}
		else
		{
			pendingFunctionCalls.push_back(
				PendingFunctionCall {
					functionCall->name,
					static_cast< int >( instructions.size() ),
					static_cast< int >(
						functionCall->arguments.size()
					)
				}
			);

			instructions.push_back(
				std::make_unique<CallFunction>(
					999 ,
					999
				)
			);
		}
	}
}


std::unique_ptr<Instruction> Compiler::instructionForOperator(
	const Token& op )
{
	switch ( op.type )
	{
	case TokenType::PLUS:
		return std::make_unique<Add>();

	case TokenType::MINUS:
		return std::make_unique<Sub>();

	case TokenType::STAR:
		return std::make_unique<Mul>();

	case TokenType::SLASH:
		return std::make_unique<Div>();

	case TokenType::LESS_THAN:
		return std::make_unique<LessThan>();

	case TokenType::GREATER_THAN:
		return std::make_unique<GreaterThan>();

	default:
		throw std::runtime_error(
			"Invalid operator"
		);
	}
}