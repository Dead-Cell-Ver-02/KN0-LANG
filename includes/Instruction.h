#pragma once 

class Instruction
{
public:
	virtual ~Instruction() = default;
};

class PushInt : public Instruction
{
public:
	int value;

	explicit PushInt( int value )
		: value( value ) {}
};

class Add : public Instruction {};
class Sub : public Instruction {};
class Mul : public Instruction {};
class Div : public Instruction {};

class LessThan : public Instruction {};
class GreaterThan : public Instruction {};

class LoadLocal : public Instruction
{
public:
	int slot;

	explicit LoadLocal( int slot )
		: slot( slot ) {}
};

class StoreLocal : public Instruction
{
public:
	int slot;

	explicit StoreLocal( int slot )
		: slot( slot ) {}
};

class JumpIfFalse : public Instruction
{
public:
	int target;

	explicit JumpIfFalse( int target )
		: target( target ) {}
};

class Jump : public Instruction
{
public:
	int target;

	explicit Jump( int target )
		: target( target ) {}
};

class CallFunction : public Instruction
{
public:
	int address;
	int arity;

	CallFunction( int address , int arity )
		: address( address ) , arity( arity ) {}
};

class Return : public Instruction {};