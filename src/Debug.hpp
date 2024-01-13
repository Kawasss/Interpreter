#pragma once
#include <string>
#include <vector>
#include "Lexer.hpp"

struct Instruction;
class StackFrame;

namespace Debug
{
	inline extern std::string DumpInstructionData(const Instruction& instruction);
	inline extern std::string DumpInstructionsData(const std::vector<Instruction>& instructions);
	inline extern std::string DumpStackFrame(const StackFrame* stackFrame);
	inline extern std::string DumpToken(Lexer::Token& token);
}