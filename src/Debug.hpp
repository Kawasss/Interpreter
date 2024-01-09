#pragma once
#include <string>
#include <vector>

struct Instruction;

namespace Debug
{
	inline extern std::string DumpInstructionData(const Instruction& instruction);
	inline extern std::string DumpInstructionsData(const std::vector<Instruction>& instructions);
}