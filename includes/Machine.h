// Machine.h
#pragma once
#include "Instruction.h"
#include <memory>
#include <optional>
#include <vector>

class Machine {
public:
	std::vector<int> run( const std::vector<std::unique_ptr<Instruction>>& instructions );

private:
	struct CallFrame {
		int returnAddress;
		std::vector<std::optional<int>> locals;
	};

	std::vector<int> stack;
	std::vector<CallFrame> callStack;

	std::optional<int> execute(
		const Instruction& instruction ,
		std::vector<std::optional<int>>& activeLocals ,
		int nextInstructionPointer
	);

	int pop();
	void storeLocal( std::vector<std::optional<int>>& locals , int slot , int value );
	int loadLocal( std::vector<std::optional<int>>& locals , int slot );
	std::vector<std::optional<int>> createFunctionLocals( int arity );
};