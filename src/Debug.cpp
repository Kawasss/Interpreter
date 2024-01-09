#include "Debug.hpp"
#include "common.hpp"

std::string Debug::DumpInstructionData(const Instruction& instruction)
{
	static constexpr int distanceBeforeOp1 = 35;
	static constexpr int distanceBeforeOp2 = 25;
	std::string ret = InstructionTypeToString(instruction.type);
	while (ret.size() < distanceBeforeOp1)
		ret.push_back(' ');

	if (instruction.operand1.dataType != DATA_TYPE_INVALID)
		ret += " " + instruction.operand1.name/* + " (" + DataTypeToString(instruction.operand1.dataType) + ")"*/;

	while (ret.size() < distanceBeforeOp1 + distanceBeforeOp2)
		ret.push_back(' ');

	if (instruction.operand2.dataType != DATA_TYPE_INVALID)
		ret += " " + instruction.operand2.name/* + " (" + DataTypeToString(instruction.operand2.dataType) + ")"*/;

	return ret;
}

std::string Debug::DumpInstructionsData(const std::vector<Instruction>& instructions)
{
	static constexpr int distanceBeforeType = 6;
	std::string ret;

	for (int i = 0; i < instructions.size(); i++)
	{
		std::string index = std::to_string(i) + ": ";
		while (index.size() < distanceBeforeType)
			index.insert(index.begin(), '0');

		ret += index + DumpInstructionData(instructions[i]) + "\n";
	}
	return ret;
}