#pragma once

#include "Expr.h"
#include "Instruction.h"
#include "stmt.h"
#include "Tokens.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct CompileResult
{
	std::vector<std::unique_ptr<Instruction>> instructions;
};

class Compiler
{
private:

	struct LocalContext
	{
		std::unordered_map<std::string , int> locals;
		int nextLocalSlot = 0;
		bool isFunctionBody = false;
	};

	struct FunctionSignature
	{
		std::vector<std::string> parameters;
		int address = -1;
	};

	struct PendingFunctionCall
	{
		std::string name;
		int instructionIndex;
		int arity;
	};

	std::unordered_map<std::string , FunctionSignature> functionSignatures;
	std::vector<PendingFunctionCall> pendingFunctionCalls;
	bool shouldLog;
	int nextUniqueNumber = 1;

	void log( const std::string& stmt );

	void patchFunctionCalls( std::vector<std::unique_ptr<Instruction>>& instructions );
	void setFunctionAddress( const std::string& name , int address );
	void emitFunctionDeclaration( const FunctionDeclaration& stmt ,
								  std::vector<std::unique_ptr<Instruction>>& instructions );
	void registerSignature( const FunctionDeclaration& stmt );
	void emit( const Stmt& stmt ,
			   std::vector<std::unique_ptr<Instruction>>& instructions ,
			   LocalContext& context );

	void emit( const Expr& expr ,
			   std::vector<std::unique_ptr<Instruction>>& instructions ,
			   LocalContext& context
	);

	std::unique_ptr<Instruction> instructionForOperator( const Token& operatorToken );

public:
	explicit Compiler( bool shouldLog = true );

	CompileResult compile( const std::vector<std::unique_ptr<Stmt>>& statements );

};