#include "Optimizer.hpp"

void Optimizer::OptimizeInstructions(std::vector<Instruction>& instructions)
{
	for (size_t i = 1; i < instructions.size(); i++)
	{
		if (instructions[i - 1].type == INSTRUCTION_TYPE_ASSIGN && instructions[i].type == INSTRUCTION_TYPE_ASSIGN && instructions[i - 1].operand1 == instructions[i].operand2) // is the previously written value immediately being read from
		{
			instructions[i].operand2 = instructions[i - 1].operand2;
			instructions.erase(instructions.begin() + i - 1);
			i--;
		}
	}
}