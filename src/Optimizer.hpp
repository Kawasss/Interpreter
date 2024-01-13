#pragma once
#include <vector>
#include "Debug.hpp"

class Optimizer
{
public:
	static void OptimizeInstructions(std::vector<Instruction>& instructions);

private:
	// pass through instructions are basically an unnecessary sequence of instruction that pass a single value along each other.
	// someting like this:
	// 
	// assign %fcv 1
	// push   %bpv %fcv
	//
	// can be optimized into:
	//
	// push %bpv 1
	//
	// it is a small difference but can save on a lot instructions depending on the context
	static bool InstructionsArePassThrough(std::vector<Instruction>& instruction, size_t index);
};