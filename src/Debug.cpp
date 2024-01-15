#include "Debug.hpp"
#include "common.hpp"
#include "StackFrame.hpp"
#include "Lexer.hpp"
#include <sstream>
#include <iomanip>

template<typename T> inline std::string ToHexadecimalString(T number)
{
	std::stringstream stream;
	stream << "0x" << std::setfill('0') << std::setw(sizeof(T) * 2) << std::hex << number;
	return stream.str();
}

std::string Debug::DumpStackFrame(const StackFrame* stackFrame)
{
	std::string ret = "Stack frame " + ToHexadecimalString(stackFrame) + " dump:\n";
	for (int i = 0; i < stackFrame->Size(); i++)
	{
		std::string currentScope = "  Scope " + std::to_string(i) + ":\n";
		const Scope& scope = stackFrame->At(i);
		if (scope.size() == 0)
			currentScope += "    None\n";
		for (const std::pair<std::string, Variable>& pair : scope)
		{
			currentScope += "    " + pair.first + " (" + DataTypeToString(pair.second.type) + ")\n";
		}
		ret += currentScope;
	}
	return ret;
}

std::string Debug::DumpInstructionData(const Instruction& instruction)
{
	static constexpr int distanceBeforeOp1 = 15;
	static constexpr int distanceBeforeOp2 = 25;

	std::string ret = InstructionTypeToString(instruction.type);
	ret = ret.substr(17, ret.size() - 17); // remove the INSTRUCTION_TYPE_ from the string, the length of that is 17

	while (ret.size() < distanceBeforeOp1)
		ret.push_back(' ');

	if (instruction.operand1.dataType != DATA_TYPE_INVALID)
		ret += " " + (instruction.operand1.literalValue.empty() ? instruction.operand1.name : instruction.operand1.literalValue)/* + " (" + DataTypeToString(instruction.operand1.dataType) + ")"*/;

	while (ret.size() < distanceBeforeOp1 + distanceBeforeOp2)
		ret.push_back(' ');

	if (instruction.operand2.dataType != DATA_TYPE_INVALID)
		ret += " " + (instruction.operand2.literalValue.empty() ? instruction.operand2.name : instruction.operand2.literalValue)/* + " (" + DataTypeToString(instruction.operand2.dataType) + ")"*/;

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

std::string Debug::DumpToken(Lexer::Token& token)
{
	static constexpr int contentLength = 12;
	static constexpr int tokenLength = 23;

	std::string ret = "line: ";
	std::string line = std::to_string(token.line);
	for (int i = 0; i < 4 - line.size(); i++) // give each string the same length indepedent of the length of the line string
		ret += '0';
	ret += line + ' ';

	ret += " content: " + token.content;
	for (int i = 0; i < contentLength - token.content.size(); i++)
		ret += ' ';

	std::string tokenAsString = LexicalTokenToString(token.token);
	ret += " token: " + tokenAsString;
	for (int i = 0; i < tokenLength - tokenAsString.size(); i++)
		ret += ' ';

	std::string lexemeAsString = LexemeToString(token.lexeme);
	ret += " lexeme: " + lexemeAsString;

	return ret;
}